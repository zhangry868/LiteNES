/*
HAL是硬件抽象层(hardware abstract layer)。
**所有**与平台相关的代码都集中在这里。
在这个实现中，我们用到了allegro游戏开发库绘制图形和监听键盘事件。

在这里实现以下抽象函数：
1) nes_hal_init()
    HAL的初始化。这个函数会在模拟器完成初始化后调用。在这里实现模拟器的
    初始化工作
2) nes_set_bg_color(r, g, b, c)
    设置背景色，r, g, b分别是红、绿、蓝分量的数值(0-255)。
    c是54色表示的内部代码，可以考虑使用
3) nes_draw_pixel(*p)
    在视频缓冲区中绘制Pixel p，其p->x, p->y是点坐标(左上角为原点，横边
    为x轴)。
    r, g, b, c为颜色信息。
4) nes_flip_display()
    将视频缓冲区内的内容显示到屏幕上，然后用设置好的背景色填充视频缓冲区。
5) wait_for_frame()
    你首先需要设置一个1/FPS (FPS=60)s的定时器。
    然后，模拟器会不断执行wait_for_frame，每一帧执行、绘制完毕后，会调用
    wait_for_frame()，在这个函数里你需要等待直到下次定时器事件触发。
    换句话说，你需要保证下面程序里的do_something()以每秒60次的频率执行：
    while (1) {
        wait_for_frame();
        do_something();
    }
6) int nes_key_state(int b) 
    询问按键状态，返回0代表松开，1代表按下。不同的b代表询问不同的按键：
    0 - Power，总是为1
    1 - 手柄上的A键
    2 - 手柄上的B键
    3 - 手柄上的SELECT键
    4 - 手柄上的START键
    5 - 手柄上的UP键
    6 - 手柄上的DOWN键
    7 - 手柄上的LEFT键
    8 - 手柄上的RIGHT键
*/
#include "nes/hal.h"
#include "nes/fce.h"
#include "nes/common.h"
#include "x86.h"
#include "device.h"


int vtx_sz = 0;
static volatile int timers = 0, live = 0;
  
/* 控制时间的函数，函数每次完成计算后会被调用，保证每秒执行FPS次 */
void wait_for_frame()
{
	int i = HZ / 60;
	while (i) {
		wait_for_interrupt();
		disable_interrupt(); // 关闭中断是为了防止数据竞争(data race)。
		if (timers > 0) {
			timers --;
			i --;
		}
		enable_interrupt();
	}
}

/* 设置背景色为(r, g, b)，等价于NES内部颜色代码c
   如果不设置，Super Mario将看到黑色而不是蓝色的背景 */
void nes_set_bg_color(int c)
{ 
    //int color = c;//CGA_Palette(r,g,b);
   clear_to_color(c);//Important
}

int CGA_Palette(int r,int g,int b)
{
     bool intense = (r > 128) | (g > 128) | (b > 128);
     bool red = (r > 123);
     bool green = (g > 122);
     bool blue = (b > 123);
     return (intense << 3)|(red << 2)|(green << 1)|(blue);
	/*int i,index = 0;
	double min = 1000000,min2;
	for(i = 0;i < 256;i ++)
	{
		min2 = ((double)(r-def_pal[3*i])/r)*((double)(r-def_pal[3*i])/r) + ((double)(g-def_pal[3*i+1])/g)*((double)(g-def_pal[3*i+1])/g) + ((double)(b-def_pal[3*i+2])/b)*((double)(b-def_pal[3*i+2])/b);
		if(min2 < (double)min)
		{	
			index = i;
			min = min2;
		}
	}
	return index;*/
	//return lpBestfitColor(def_pal,r>>1,g>>1,b>>1);
}


/* 在屏幕上绘制一个像素点
   但在这个实现里，我们只是保存这个像素点，在flip_display时统一绘制 */
void nes_draw_pixel(Pixel *p)
{ 
    int color = p->c;//CGA_Palette(p->r,p->g,p->b);//(int)(0.299*p->r + 0.587*p->g + 0.114*p->b);//,g,b;
    draw_pixel(p->x * 5/4, p->y * 5/6, color);
    if(!(p->x & 0x3))
   	draw_pixel(p->x * 5/4 -1, p->y * 5/6, color);
    //unsigned long xrIntFloat_16=(213<<16)/256+1; //16.16格式定点数
   // unsigned long yrIntFloat_16=(200<<16)/240+1; //16.16格式定点数
    //draw_pixel((xrIntFloat_16 * p->x) >>16,(p->y)*yrIntFloat_16 >> 16,color);
    //draw_string("Hello World!",0,SCR_HEIGHT - 8,10);
    /*  ALLEGRO_VERTEX px;
    px.x = p->x;
    px.y = p->y;
    px.z = 0;
    px.color = al_map_rgb(p->r, p->g, p->b);
    memcpy(&vtx[vtx_sz++], &px, sizeof(ALLEGRO_VERTEX));*/
}
void
handle_timer(void) { // 时钟中断处理函数
	timers ++;
}
/* 初始化一切需要的内容。
   在这里主要是调用allegro库，初始化定时器、键盘和屏幕 */
void nes_hal_init()
{
	int i;
	out_byte(0x03c8, 0);
	for(i = 0;i < 64;i++)
	{
		out_byte(0x03c9, (palette[i].r) >>2);
		out_byte(0x03c9, (palette[i].g) >>2);
		out_byte(0x03c9, (palette[i].b) >>2);
	}
}

/* 这个函数每秒会被调用FPS次，每次调用时更新屏幕内容 */
void nes_flip_display()
{
    /*
    al_draw_prim(vtx, NULL, NULL, 0, vtx_sz, ALLEGRO_PRIM_POINT_LIST);
     al_flip_display();
    vtx_sz = 0;*/
}

/* 询问编码为b的按键是否被按下(返回1为按下) */
int nes_key_state(int b)
{
    switch (b)
	{
        case 0: // On / Off
            return 1;
        case 1: // A
            return GetKey(37);//K
        case 2: // B
            return GetKey(36);;//J
        case 3: // SELECT
            return GetKey(22);//U
        case 4: // START
            return GetKey(23);//I
        case 5: // UP
            return GetKey(17);//W
        case 6: // DOWN
            return GetKey(31);//S
        case 7: // LEFT
            return GetKey(30);//A
        case 8: // RIGHT
            return GetKey(32);//D
        default:
            return 1;
    }
}

