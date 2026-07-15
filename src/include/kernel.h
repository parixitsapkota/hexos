#include <int.h>

extern u8 *g_framebuffer;
extern u16 g_pitch;
extern u16 g_height;
extern u16 g_width;
extern u8 g_bpp;

typedef struct {
  u8 *g_framebuffer;
  u16 g_pitch;
  u16 g_height;
  u16 g_width;
  u8 g_bpp;
} BootInfo;

typedef void (*KernelEntry)(BootInfo*);
