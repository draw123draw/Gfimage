#ifndef HEADERS_H
#define HEADERS_H
#include<FL/Fl.H>
#include<FL/Fl_Window.H>
#include<FL/Fl_Gl_Window.H>
#include<GL/gl.h>

#include<FL/Fl_Box.H>
#include<FL/Fl_Button.H>
#include<FL/Fl_Input.H>
#include<FL/Fl_Menu_Bar.H>
#include<FL/Fl_Value_Slider.H>
#include<FL/Fl_Counter.H>
#include<FL/Fl_Int_Input.H>
#include<FL/Fl_Text_Editor.H>
#include<FL/Fl_Input_Choice.H>
#include<FL/Fl_Native_File_Chooser.H>
#include<FL/Fl_Color_Chooser.H>
#include<FL/Fl_Chart.H>
#include<FL/Fl_Table_Row.H>
#include<FL/Fl_Tabs.H>
#include<FL/Fl_Tooltip.H>

#include<FL/fl_ask.H>
#include<FL/fl_draw.H>
#include<math.h>
#include<float.h>
#include"defs.h"

#ifdef _WIN32
#define fseeko64 _fseeki64
#define ftello64 _ftelli64
#endif

Fl_RGB_Image *ico=new Fl_RGB_Image(idata,32,32,3,0);
struct manynames
{
	char basename[128];
	char path[256];
	char path_slash[256];
	char base[128];
	char ext[16];
};
class ColorBar;
class ColorPoint;
class Imagesc;
class FreeCounter;
class FreeCounterHeader;
class App;
class HeaderTable;
class VHeaderTable;

class ColorPoint:public Fl_Box
{
public:
    int idx;
    ColorPoint(int x,int y,int w,int h,double r_,double g_,double b_,ColorBar *colorbar_);
    int handle(int event)override;
};

class ColorBar:public Fl_Gl_Window//在Colorbar窗口中
{
public:
    int cbox_s=5,cbox_height=35;
    float *fs;
    int c_num=0;
    ColorBar(int x,int y,int w,int h);
    ~ColorBar();
    void set_colormap(const char *color_idx);
    void set_color_points(int color_num,const float *defalut_clr);
    void get_rgb(float f01,double &r,double &g,double &b);
    void remove_color_point(int idx);
    void sort_colors();
    void draw()override;
    int handle(int event)override;
};

// class

class Imagesc:public Fl_Gl_Window
{
public:
    float *mat;
    float max_a,min_a,max_abs_a=0;
    float max_s,min_s,max_abs_s=0;
    bool set_mat=false;
    bool centered_norm=false;
    int rows,cols;
    void draw()override;
    Imagesc(int x,int y,int width,int height);
    void set_window_para(float *mat_f,int rows_f,int cols_f);
    int handle(int event)override;
};

class FreeCounter:public Fl_Counter
{
    static void tc_cb(Fl_Widget *w,void *data);
    static void ok_cb(Fl_Widget *w,void *data);
public:
    int step_s,step_l;
    Fl_Window *counter_set;
    Fl_Int_Input *fi_curr;
    Fl_Int_Input *fi_step_s;
    Fl_Int_Input *fi_step_l;
    Fl_Button *set_ok;
    FreeCounter(int x,int y,int w,int h);
    int handle(int event)override;
};

class FreeCounterHeader:public Fl_Counter
{
    static void tc_cb(Fl_Widget *w,void *data);
public:
    Fl_Window *counter_set;
    Fl_Int_Input *fi_curr;
    Fl_Button *set_ok;
    FreeCounterHeader(int X,int Y,int W,int H,const char *title);
    int handle(int event)override;
};

class App:public Fl_Window
{
    int open_x=120;//打开文件按钮
    int open_y=50;

    unsigned char hdr[3600];
    char mxy[64],amp_c[64];
    bool first_read=true;
    bool is_le;
    bool is_pc;
    Fl_Menu_Bar *menus;
    Fl_Text_Editor *hdr_text;
    Fl_Text_Buffer *tbuff;
    Fl_Button* open_button;
    Fl_Button *save_hdr;

    Fl_Window* CroppingWin=NULL;
    Fl_Window* SaveWin=NULL;
    Fl_Window* TextWin=NULL;
    Fl_Window* HdrWin=NULL;
    Fl_Window *PropertyWin=NULL;
    Fl_Native_File_Chooser *fc,*fcc,*fcd;
    Fl_Input_Choice *colormap_c;
    static void start_cb(Fl_Widget* widget,void *);
    static void ts_cb(Fl_Widget *widget,void *);
    static void save_cb(Fl_Widget *widget,void *);
    static void property_cb(Fl_Widget *widget,void *);
    static void hdr_cb(Fl_Widget *widget,void *);
    static void cutter_separater_cb(Fl_Widget *widget,void *);
    static void format_cb(Fl_Widget *widget,void *);
    static void endian_cb(Fl_Widget *widget,void *);
    static void enhance_cb(Fl_Widget *widget,void *);
    static void attenua_cb(Fl_Widget *widget,void *);
    static void load_clr_cb(Fl_Widget *widget,void *);
    static void save_clr_cb(Fl_Widget *widget,void *);
    static void save_sgy_data_cb(Fl_Widget *w,void *);
public:
    char filename[100];
    struct manynames filenames;
    int *histo;
    FILE *fpi,*fpo,*fpd;
    long long traces;
    int trace_s,samples,enhance_cnt=0;
    float *seis=NULL;
    long long trace_num=0;

    int height_of_menu=30;
    Fl_Chart *chart=NULL;
    const int bin_num=60;
    Fl_Window *ColorbarWin=NULL;
    Fl_Box *where_mouse_box=NULL;
    Fl_Value_Slider *trace_slider;
    FreeCounter *trace_counter;
    Fl_Int_Input *n_parts_input;
    Fl_Int_Input *n_traces_input;
    Fl_Int_Input *start_trace_input;
    Fl_Int_Input *end_trace_input;
    Fl_Int_Input *interval_trace_input;
    Fl_Int_Input *start_sample_input;
    Fl_Int_Input *end_sample_input;
    Fl_Int_Input *interval_sample_input;

    App(int X,int Y,int Width,int Height,const char *title);
    ~App();
    int handle(int event)override;
    void swap_bytes();
    void open_figure();
    void change_profile();
    void read_data(const char *fname);
    void ieee2ibm();//公式已经考虑了字节序，无需再用swap_bytes转换
};

class HeaderTable:public Fl_Table_Row
{
    void draw_cell(TableContext context,int ROW=0,int COL=0,int X=0,int Y=0,int W=0,int H=0)override;
    void DrawHeader(const char *s,int X,int Y,int W,int H);
public:
    int trace_s_h=5;
    int selected_R=1;
    int bytes_num=4;
    long long trace_num=0;
    unsigned char *hdrbytes_s;
    void get_hdr();
    HeaderTable(int X,int Y,int W,int H,const char *title);
    ~HeaderTable();
    int handle(int event)override;
};

class VHeaderTable:public Fl_Table
{
    void draw_cell(TableContext context,int ROW=0,int COL=0,int X=0,int Y=0,int W=0,int H=0)override;
public:
    VHeaderTable(int X,int Y,int W,int H,const char *title);
    unsigned char vhdrbytes[3200];
};

class ResultTable:public Fl_Table
{
    void draw_cell(TableContext context,int ROW=0,int COL=0,int X=0,int Y=0,int W=0,int H=0)override;
public:
    ResultTable(int X,int Y,int W,int H,const char *title);
    int b2i(int COL);
    bool is_le=false;
};

App *app;
ColorPoint **colors=nullptr;
ColorPoint *tmp_c=nullptr;
ColorBar *colorbar=nullptr;
Imagesc *fwin=nullptr;
HeaderTable *headertable=nullptr;
VHeaderTable *vht=nullptr;
ResultTable *rt=nullptr;
void findnames(char* filename,manynames* outfile);

#endif