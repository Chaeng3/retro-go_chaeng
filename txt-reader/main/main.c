#include <rg_system.h>
#include <rg_gui.h>
#include <rg_input.h>
#include <rg_storage.h>
#include <rg_utils.h>
#include <rg_display.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern const rg_font_t font_ebook;      // Source Han 16px
extern const rg_font_t font_ebook_20;   // Source Han 20px
extern const rg_font_t font_ebook_24;   // Source Han 24px

#define GBK_LEAD_BASE   0x81
#define GBK_LEAD_COUNT  126
#define GBK_TRAIL_BASE  0x40
#define GBK_TRAIL_COUNT 191
extern const uint16_t gbk_unicode[GBK_LEAD_COUNT][GBK_TRAIL_COUNT];

#define CHUNK_SIZE      4096
#define PAGE_TEXT_SIZE  4096
#define HISTORY_MAX     256
#define SLOT_COUNT      4
#define SLOT_EMPTY      0xFFFFFFFFu
#define TEXT_MARGIN_X   10

// Native outline-rasterized sizes only — never stretch a bitmap font.
static const rg_font_t *const font_size_steps[] = {
    &font_ebook,
    &font_ebook_20,
    &font_ebook_24,
};
static int font_size_idx = 0;

typedef enum
{
    BOOK_ENC_UTF8 = 0,
    BOOK_ENC_GBK = 1,
} book_encoding_t;

static book_encoding_t book_enc = BOOK_ENC_UTF8;

static void apply_reader_font(void)
{
    rg_gui_set_font_ptr(font_size_steps[font_size_idx]);
}

static bool change_font_size(int delta)
{
    int next = font_size_idx + delta;
    int max = (int)(sizeof(font_size_steps) / sizeof(font_size_steps[0])) - 1;
    if (next < 0 || next > max)
        return false;
    font_size_idx = next;
    apply_reader_font();
    return true;
}

static int text_line_height(void)
{
    // TEXT_RECT includes 1px padding top+bottom; we draw with NO_PADDING.
    return RG_MAX(1, TEXT_RECT("A", 0).height - 2);
}

static rg_app_t *app;
static FILE *book_fp;
static size_t book_size;
static size_t page_offset;
static size_t next_offset;
static char page_text[PAGE_TEXT_SIZE];
static size_t history[HISTORY_MAX];
static int history_len;
static bool dirty_progress;

// Four fixed slots — same idea as game savestates (Slot 0..3).
static uint32_t slot_offsets[SLOT_COUNT] = {
    SLOT_EMPTY, SLOT_EMPTY, SLOT_EMPTY, SLOT_EMPTY
};

static void save_progress(void);
static void refresh_slot_cache(void);
static void jump_percent(int percent);
static void jump_offset(size_t offset);
static void draw_page(void);
static int find_slot_on_page(void);
static void options_handler(rg_gui_option_t *dest);

// Same as game savestates: path is chosen by the framework (Resume / Save).
static bool load_state_handler(const char *filename)
{
    void *data = NULL;
    size_t len = 0;
    uint32_t offset = 0;

    if (!filename || !rg_storage_read_file(filename, &data, &len, 0) || !data || len < sizeof(offset))
    {
        free(data);
        return false;
    }
    memcpy(&offset, data, sizeof(offset));
    free(data);
    if (book_size > 0 && offset >= book_size)
        return false;
    page_offset = offset;
    history_len = 0;
    dirty_progress = true;
    return true;
}

static bool save_state_handler(const char *filename)
{
    uint32_t offset = (uint32_t)page_offset;

    if (!filename)
        return false;
    rg_storage_mkdir(rg_dirname(filename));
    return rg_storage_write_file(filename, &offset, sizeof(offset), 0);
}

static bool reset_handler(bool hard)
{
    (void)hard;
    page_offset = 0;
    history_len = 0;
    dirty_progress = true;
    return true;
}

static bool screenshot_handler(const char *filename, int width, int height)
{
    (void)filename;
    (void)width;
    (void)height;
    return false;
}

static void event_handler(int event, void *arg)
{
    (void)arg;
    if (event == RG_EVENT_SHUTDOWN || event == RG_EVENT_SLEEP)
        save_progress();
    else if (event == RG_EVENT_REDRAW)
        draw_page();
}

static rg_gui_event_t jump_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_ENTER)
    {
        jump_percent((int)option->arg);
        // Close this dialog so the new page is visible after leaving Options.
        return RG_DIALOG_SELECT;
    }
    return RG_DIALOG_VOID;
}

// Game-compatible slot paths: /saves/txt/<book>.txt.sav, -1.sav, …
// Launcher Resume/New game uses rg_emu_get_states() on these same paths.
static char *ebook_slot_path(int slot)
{
    if (slot < 0 || slot >= SLOT_COUNT || !app || !app->romPath)
        return NULL;
    return rg_emu_get_path(RG_PATH_SAVE_STATE + slot, app->romPath);
}

// Older builds used CRC filenames; migrate so Resume can see them.
static char *ebook_legacy_slot_path(int slot)
{
    char *path;
    uint32_t id;
    const char *p;

    if (slot < 0 || slot >= SLOT_COUNT)
        return NULL;
    path = malloc(RG_PATH_MAX);
    if (!path)
        return NULL;
    p = app && app->romPath ? app->romPath : "";
    id = rg_crc32(0, (const uint8_t *)p, strlen(p));
    if (slot <= 0)
        snprintf(path, RG_PATH_MAX, "%s/txt/bmk_%08x.sav", RG_BASE_PATH_SAVES, (unsigned)id);
    else
        snprintf(path, RG_PATH_MAX, "%s/txt/bmk_%08x-%d.sav", RG_BASE_PATH_SAVES, (unsigned)id, slot);
    return path;
}

static char *ebook_progress_path(void)
{
    char *path = malloc(RG_PATH_MAX);
    uint32_t id;
    const char *p;

    if (!path)
        return NULL;
    p = app && app->romPath ? app->romPath : "";
    id = rg_crc32(0, (const uint8_t *)p, strlen(p));
    snprintf(path, RG_PATH_MAX, "%s/txt/bmk_%08x.pos", RG_BASE_PATH_SAVES, (unsigned)id);
    return path;
}

static bool read_slot_offset(const char *path, uint32_t *out)
{
    void *data = NULL;
    size_t len = 0;
    uint32_t offset = 0;

    if (!path || !out)
        return false;
    if (!rg_storage_read_file(path, &data, &len, 0) || !data || len < sizeof(offset))
    {
        free(data);
        return false;
    }
    memcpy(&offset, data, sizeof(offset));
    free(data);
    if (book_size > 0 && offset >= book_size)
        return false;
    *out = offset;
    return true;
}

static bool migrate_legacy_slot(int slot, const char *dest)
{
    char *legacy = ebook_legacy_slot_path(slot);
    uint32_t offset = 0;
    bool ok = false;

    if (!legacy || !dest)
    {
        free(legacy);
        return false;
    }
    if (read_slot_offset(legacy, &offset))
    {
        rg_storage_mkdir(rg_dirname(dest));
        if (rg_storage_write_file(dest, &offset, sizeof(offset), 0))
        {
            remove(legacy);
            ok = true;
        }
    }
    free(legacy);
    return ok;
}

static void options_handler(rg_gui_option_t *dest)
{
    // Shown under Options → Emulator options (same hook as other cores).
    *dest++ = (rg_gui_option_t){0, _("Jump to start"), NULL, RG_DIALOG_FLAG_NORMAL, &jump_cb};
    *dest++ = (rg_gui_option_t){25, _("Jump 25%"), NULL, RG_DIALOG_FLAG_NORMAL, &jump_cb};
    *dest++ = (rg_gui_option_t){50, _("Jump 50%"), NULL, RG_DIALOG_FLAG_NORMAL, &jump_cb};
    *dest++ = (rg_gui_option_t){75, _("Jump 75%"), NULL, RG_DIALOG_FLAG_NORMAL, &jump_cb};
    *dest++ = (rg_gui_option_t){100, _("Jump to end"), NULL, RG_DIALOG_FLAG_NORMAL, &jump_cb};
    *dest++ = (rg_gui_option_t)RG_DIALOG_END;
}

// Auto last-read position (not one of the 4 bookmark slots).
static char *progress_path(void)
{
    return ebook_progress_path();
}

static void refresh_slot_cache(void)
{
    for (int i = 0; i < SLOT_COUNT; ++i)
    {
        char *path = ebook_slot_path(i);
        uint32_t offset = SLOT_EMPTY;

        if (path)
        {
            if (!read_slot_offset(path, &offset))
            {
                if (migrate_legacy_slot(i, path))
                    read_slot_offset(path, &offset);
                else
                    offset = SLOT_EMPTY;
            }
        }
        free(path);
        slot_offsets[i] = offset;
    }
}

static void save_progress(void)
{
    char *path = progress_path();
    uint32_t offset = (uint32_t)page_offset;

    if (!path)
        return;
    rg_storage_mkdir(RG_BASE_PATH_SAVES "/txt");
    rg_storage_mkdir(rg_dirname(path));
    rg_storage_write_file(path, &offset, sizeof(offset), 0);
    free(path);
    dirty_progress = false;
}

static int find_slot_on_page(void)
{
    for (int i = 0; i < SLOT_COUNT; ++i)
    {
        uint32_t off = slot_offsets[i];
        if (off == SLOT_EMPTY)
            continue;
        if (off >= page_offset && off < next_offset)
            return i;
        if (off == page_offset)
            return i;
    }
    return -1;
}

static size_t utf8_prev_boundary(const char *buf, size_t len)
{
    while (len > 0 && (buf[len - 1] & 0xC0) == 0x80)
        len--;
    return len;
}

static size_t gbk_prev_boundary(const uint8_t *buf, size_t len)
{
    size_t i = 0;
    size_t last_complete = 0;
    while (i < len)
    {
        if (buf[i] < 0x80)
        {
            i++;
            last_complete = i;
        }
        else if (i + 1 < len)
        {
            i += 2;
            last_complete = i;
        }
        else
            break;
    }
    return last_complete;
}

static bool utf8_buffer_looks_valid(const uint8_t *data, size_t len)
{
    size_t i = 0;
    size_t bad = 0;
    size_t checked = 0;
    while (i < len)
    {
        uint8_t c = data[i];
        if (c < 0x80)
        {
            i++;
            continue;
        }
        checked++;
        int need = 0;
        if ((c & 0xE0) == 0xC0)
            need = 1;
        else if ((c & 0xF0) == 0xE0)
            need = 2;
        else if ((c & 0xF8) == 0xF0)
            need = 3;
        else
        {
            bad++;
            i++;
            continue;
        }
        if (i + need >= len)
            break;
        bool ok = true;
        for (int k = 1; k <= need; ++k)
        {
            if ((data[i + k] & 0xC0) != 0x80)
            {
                ok = false;
                break;
            }
        }
        if (!ok)
            bad++;
        i += 1 + need;
    }
    if (checked == 0)
        return true; // pure ASCII → treat as UTF-8
    return bad * 10 < checked; // allow a little noise
}

static void detect_book_encoding(void)
{
    uint8_t sample[4096];
    size_t n;

    book_enc = BOOK_ENC_UTF8;
    if (!book_fp)
        return;
    fseek(book_fp, 0, SEEK_SET);
    n = fread(sample, 1, sizeof(sample), book_fp);
    if (n >= 3 && sample[0] == 0xEF && sample[1] == 0xBB && sample[2] == 0xBF)
    {
        book_enc = BOOK_ENC_UTF8;
        return;
    }
    if (!utf8_buffer_looks_valid(sample, n))
        book_enc = BOOK_ENC_GBK;
}

static int decode_one(const uint8_t *p, size_t avail, int *out_cp)
{
    if (avail == 0)
        return 0;

    if (book_enc == BOOK_ENC_GBK)
    {
        uint8_t c = p[0];
        if (c < 0x80)
        {
            *out_cp = c;
            return 1;
        }
        if (avail < 2)
            return 0;
        uint8_t lead = p[0];
        uint8_t trail = p[1];
        if (lead < GBK_LEAD_BASE || lead > 0xFE || trail < GBK_TRAIL_BASE || trail == 0x7F || trail > 0xFE)
        {
            *out_cp = 0xFFFD;
            return 1;
        }
        uint16_t u = gbk_unicode[lead - GBK_LEAD_BASE][trail - GBK_TRAIL_BASE];
        *out_cp = u ? u : 0xFFFD;
        return 2;
    }

    const char *ptr = (const char *)p;
    const char *start = ptr;
    int chr = rg_utf8_decode(&ptr);
    if (chr < 0)
        return 0;
    *out_cp = chr;
    return (int)(ptr - start);
}

static int measure_glyph_width(int codepoint)
{
    char tmp[8];
    size_t n = rg_utf8_encode(tmp, codepoint);
    tmp[n] = 0;
    // TEXT_RECT includes 1px padding on each side; drawing uses glyph advance only.
    int w = TEXT_RECT(tmp, 0).width;
    return RG_MAX(1, w - 2);
}

static void build_page(void)
{
    static uint8_t chunk[CHUNK_SIZE];
    int screen_w = rg_display_get_width();
    int screen_h = rg_display_get_height();
    int line_h = text_line_height();
    int max_lines = RG_MAX(1, (screen_h - line_h) / line_h);
    int max_w = RG_MAX(8, screen_w - TEXT_MARGIN_X * 2);
    size_t nread;
    size_t pos = 0;
    size_t page_len = 0;
    int lines = 0;
    int line_w = 0;

    page_text[0] = 0;
    next_offset = page_offset;

    if (!book_fp || page_offset >= book_size)
        return;

    fseek(book_fp, (long)page_offset, SEEK_SET);
    nread = fread(chunk, 1, CHUNK_SIZE - 1, book_fp);
    if (book_enc == BOOK_ENC_UTF8)
        nread = utf8_prev_boundary((char *)chunk, nread);
    else
        nread = gbk_prev_boundary(chunk, nread);
    chunk[nread] = 0;

    if (page_offset == 0 && book_enc == BOOK_ENC_UTF8 && nread >= 3 &&
        chunk[0] == 0xEF && chunk[1] == 0xBB && chunk[2] == 0xBF)
        pos = 3;

    while (pos < nread && lines < max_lines && page_len + 8 < PAGE_TEXT_SIZE)
    {
        size_t prev = pos;
        int chr = 0;
        int consumed = decode_one(chunk + pos, nread - pos, &chr);
        if (consumed <= 0)
            break;
        pos += (size_t)consumed;

        if (chr == '\r')
            continue;

        if (chr == '\n')
        {
            page_text[page_len++] = '\n';
            lines++;
            line_w = 0;
            continue;
        }

        int gw = measure_glyph_width(chr);
        if (gw <= 0)
            gw = 8;

        if (line_w + gw > max_w && line_w > 0)
        {
            page_text[page_len++] = '\n';
            lines++;
            line_w = 0;
            if (lines >= max_lines)
            {
                pos = prev;
                break;
            }
        }

        char utf[8];
        size_t ub = rg_utf8_encode(utf, chr);
        if (page_len + ub >= PAGE_TEXT_SIZE - 1)
        {
            pos = prev;
            break;
        }
        memcpy(page_text + page_len, utf, ub);
        page_len += ub;
        line_w += gw;
    }

    page_text[page_len] = 0;
    next_offset = page_offset + pos;
    if (next_offset > book_size)
        next_offset = book_size;
}

static void history_push(size_t offset)
{
    if (history_len > 0 && history[history_len - 1] == offset)
        return;
    if (history_len >= HISTORY_MAX)
    {
        memmove(history, history + 1, (HISTORY_MAX - 1) * sizeof(history[0]));
        history_len = HISTORY_MAX - 1;
    }
    history[history_len++] = offset;
}

static void go_next_page(void)
{
    if (next_offset >= book_size)
        return;
    history_push(page_offset);
    page_offset = next_offset;
    dirty_progress = true;
}

static void go_prev_page(void)
{
    if (history_len > 0)
    {
        page_offset = history[--history_len];
        dirty_progress = true;
        return;
    }
    if (page_offset == 0)
        return;
    page_offset = page_offset > CHUNK_SIZE / 2 ? page_offset - CHUNK_SIZE / 2 : 0;
    dirty_progress = true;
}

static void jump_offset(size_t offset)
{
    history_len = 0;
    if (offset >= book_size)
        offset = book_size > 0 ? book_size - 1 : 0;
    page_offset = offset;

    if (page_offset > 0 && book_fp)
    {
        if (book_enc == BOOK_ENC_UTF8)
        {
            char probe[4];
            fseek(book_fp, (long)page_offset, SEEK_SET);
            size_t n = fread(probe, 1, 3, book_fp);
            while (page_offset < book_size && n > 0 && (probe[0] & 0xC0) == 0x80)
            {
                page_offset++;
                fseek(book_fp, (long)page_offset, SEEK_SET);
                n = fread(probe, 1, 1, book_fp);
            }
        }
        else if (page_offset > 0)
        {
            // If we landed on a GBK trail byte, step back to the lead.
            uint8_t pair[2];
            fseek(book_fp, (long)(page_offset - 1), SEEK_SET);
            if (fread(pair, 1, 2, book_fp) == 2 &&
                pair[0] >= 0x81 && pair[0] <= 0xFE &&
                pair[1] >= 0x40 && pair[1] != 0x7F && pair[1] <= 0xFE)
            {
                page_offset -= 1;
            }
        }
    }
    dirty_progress = true;
}

static void jump_percent(int percent)
{
    size_t offset;
    if (percent <= 0)
        offset = 0;
    else if (percent >= 100)
        offset = book_size > 0 ? book_size - 1 : 0;
    else
        offset = (size_t)((book_size * (size_t)percent) / 100);
    jump_offset(offset);
}

static void jump_by_percent(int delta)
{
    size_t step;

    if (book_size == 0 || delta == 0)
        return;
    step = book_size / 20; // 5%
    if (step == 0)
        step = 1;
    if (delta < 0)
    {
        if (page_offset <= step)
            jump_offset(0);
        else
            jump_offset(page_offset - step);
    }
    else
    {
        if (page_offset >= book_size - 1)
            return;
        jump_offset(page_offset + step);
    }
}

static void draw_page(void)
{
    int screen_w = rg_display_get_width();
    int screen_h = rg_display_get_height();
    int line_h = text_line_height();
    char footer[40];
    int percent = book_size ? (int)((page_offset * 100) / book_size) : 0;
    bool marked = false;

    build_page();
    marked = find_slot_on_page() >= 0;

    rg_gui_draw_rect(0, 0, screen_w, screen_h, 0, C_NONE, C_BLACK);
    rg_gui_draw_text(TEXT_MARGIN_X, 0, screen_w - TEXT_MARGIN_X * 2, page_text, C_WHITE, C_BLACK,
        RG_TEXT_MULTILINE | RG_TEXT_NO_PADDING);

    rg_gui_draw_rect(0, screen_h - line_h, screen_w, line_h, 0, C_NONE, C_BLACK);
    if (marked)
        snprintf(footer, sizeof(footer), "* %d%%", percent);
    else
        snprintf(footer, sizeof(footer), "%d%%", percent);
    rg_gui_draw_text(0, screen_h - line_h, screen_w, footer, C_SILVER, C_BLACK,
        RG_TEXT_ALIGN_CENTER | RG_TEXT_NO_PADDING);
}

void app_main(void)
{
    const rg_handlers_t handlers = {
        .loadState = &load_state_handler,
        .saveState = &save_state_handler,
        .reset = &reset_handler,
        .screenshot = &screenshot_handler,
        .event = &event_handler,
        .options = &options_handler,
    };

    uint32_t prev_keys = 0;
    rg_stat_t st;

    app = rg_system_init(32000, &handlers, NULL);
    rg_system_set_tick_rate(30);
    // GNU Unifont 16px (large CJK coverage); size keys scale from 16..32
    apply_reader_font();

    if (!app->romPath || !app->romPath[0])
    {
        rg_gui_alert(_("E-Book"), _("No file selected."));
        rg_system_exit();
    }

    st = rg_storage_stat(app->romPath);
    if (!st.exists || !st.is_file)
    {
        rg_gui_alert(_("E-Book"), _("Failed to open file."));
        rg_system_exit();
    }

    book_fp = fopen(app->romPath, "rb");
    if (!book_fp)
    {
        rg_gui_alert(_("E-Book"), _("Failed to open file."));
        rg_system_exit();
    }

    book_size = st.size;
    detect_book_encoding();
    page_offset = 0;
    history_len = 0;
    refresh_slot_cache();

    // Same boot contract as emulators: Resume loads a slot; New game starts at 0.
    if (app->bootFlags & RG_BOOT_RESUME)
        rg_emu_load_state(app->saveSlot);
    dirty_progress = false;

    draw_page();

    while (1)
    {
        uint32_t keys = rg_input_read_gamepad();
        uint32_t pressed = keys & ~prev_keys;
        prev_keys = keys;
        bool redraw = false;

        // Same as emulators: Menu = game menu, Option = options menu
        if (pressed & RG_KEY_MENU)
        {
            save_progress();
            rg_gui_game_menu();
            apply_reader_font();
            refresh_slot_cache();
            redraw = true;
        }
        else if (pressed & RG_KEY_OPTION)
        {
            rg_gui_options_menu();
            apply_reader_font();
            refresh_slot_cache();
            redraw = true;
        }
        // Select: larger font; Start: smaller font
        else if (pressed & RG_KEY_SELECT)
        {
            if (change_font_size(+1))
                redraw = true;
        }
        else if (pressed & RG_KEY_START)
        {
            if (change_font_size(-1))
                redraw = true;
        }
        // Up / A: previous page; Down / B: next page
        else if (pressed & (RG_KEY_UP | RG_KEY_A))
        {
            go_prev_page();
            redraw = true;
        }
        else if (pressed & (RG_KEY_DOWN | RG_KEY_B))
        {
            go_next_page();
            redraw = true;
        }
        // Left / Right: jump ±5% through the book
        else if (pressed & RG_KEY_LEFT)
        {
            jump_by_percent(-5);
            redraw = true;
        }
        else if (pressed & RG_KEY_RIGHT)
        {
            jump_by_percent(+5);
            redraw = true;
        }

        if (redraw)
        {
            draw_page();
            if (dirty_progress)
                save_progress();
        }

        rg_system_tick(1000000 / 30);
        rg_task_delay(16);
    }
}
