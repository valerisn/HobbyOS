#include "input.h"
#include "lib.h"

int mouse_x, mouse_y, mouse_btn;

static int scr_w, scr_h;
static int shift, ext;
static u8 pkt[3];
static int pkt_n;
static char keyq[32];
static int kq_head, kq_tail;

/* set 1 scancodes, the controller translates for us (see input_init) */
static const char map_n[] = "\0\x1b" "1234567890-=" "\b\t" "qwertyuiop[]" "\n\0" "asdfghjkl;'`" "\0\\" "zxcvbnm,./" "\0*\0 ";
static const char map_s[] = "\0\x1b" "!@#$%^&*()_+" "\b\t" "QWERTYUIOP{}" "\n\0" "ASDFGHJKL:\"~" "\0|" "ZXCVBNM<>?" "\0*\0 ";

static void kbc_wait_write(void)
{
    for (int i = 0; i < 100000 && (inb(0x64) & 2); i++) ;
}

static int kbc_read(void)
{
    for (int i = 0; i < 100000; i++)
        if (inb(0x64) & 1) return inb(0x60);
    return -1;
}

static void kbc_cmd(u8 c) { kbc_wait_write(); outb(0x64, c); }
static void kbc_data(u8 d) { kbc_wait_write(); outb(0x60, d); }

void input_init(int w, int h)
{
    scr_w = w; scr_h = h;
    mouse_x = w / 2; mouse_y = h / 2;

    while (inb(0x64) & 1) inb(0x60);

    kbc_cmd(0xA8);
    kbc_cmd(0x20);
    int cfg = kbc_read();
    if (cfg < 0) cfg = 0x45;
    /* irqs off since we poll, both clocks on, translation on so the tables above hold */
    cfg = (cfg & ~0x33) | 0x40;
    kbc_cmd(0x60);
    kbc_data(cfg);

    kbc_cmd(0xAE);
    kbc_data(0xF4);
    kbc_read();

    kbc_cmd(0xD4);
    kbc_data(0xF6);
    kbc_read();
    kbc_cmd(0xD4);
    kbc_data(0xF4);
    kbc_read();

    while (inb(0x64) & 1) inb(0x60);
}

static void key_scan(u8 sc)
{
    if (sc == 0xE0) { ext = 1; return; }
    if (ext) { ext = 0; return; }

    int up = sc & 0x80;
    sc &= 0x7f;
    if (sc == 0x2A || sc == 0x36) { shift = !up; return; }
    if (up || sc >= sizeof(map_n) - 1) return;

    char c = shift ? map_s[sc] : map_n[sc];
    if (!c) return;
    int next = (kq_head + 1) % 32;
    if (next != kq_tail) { keyq[kq_head] = c; kq_head = next; }
}

static int mouse_byte(u8 b)
{
    if (pkt_n == 0 && !(b & 8)) return 0;
    pkt[pkt_n++] = b;
    if (pkt_n < 3) return 0;
    pkt_n = 0;

    int dx = pkt[1] - ((pkt[0] & 0x10) ? 256 : 0);
    int dy = pkt[2] - ((pkt[0] & 0x20) ? 256 : 0);
    mouse_x += dx;
    mouse_y -= dy;
    if (mouse_x < 0) mouse_x = 0;
    if (mouse_y < 0) mouse_y = 0;
    if (mouse_x >= scr_w) mouse_x = scr_w - 1;
    if (mouse_y >= scr_h) mouse_y = scr_h - 1;
    mouse_btn = pkt[0] & 7;
    return 1;
}

int input_poll(void)
{
    int changed = 0;
    int budget = 64;
    u8 st;
    while (budget-- && ((st = inb(0x64)) & 1)) {
        u8 d = inb(0x60);
        if (st & 0x20) {
            changed |= mouse_byte(d);
        } else {
            int before = kq_head;
            key_scan(d);
            changed |= (before != kq_head);
        }
    }
    return changed;
}

int input_key(void)
{
    if (kq_head == kq_tail) return 0;
    char c = keyq[kq_tail];
    kq_tail = (kq_tail + 1) % 32;
    return c;
}
