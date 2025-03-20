#ifndef HEADERS_H
#define HEADERS_H
#include<FL/Fl.H>
#include<FL/Fl_Window.H>
#include<FL/Fl_Gl_Window.H>
#include<GL/gl.h>
#include<FL/gl.h>

#include<FL/Fl_Box.H>
#include<FL/Fl_Button.H>
#include<FL/Fl_Input.H>
#include<FL/Fl_Menu_Bar.H>
#include<FL/Fl_Hor_Slider.H>
#include<FL/Fl_Counter.H>
#include<FL/Fl_Int_Input.H>
#include<FL/Fl_Text_Editor.H>
#include<FL/Fl_Input_Choice.H>
#include<FL/Fl_Native_File_Chooser.H>
#include<FL/Fl_Color_Chooser.H>
#include<FL/Fl_Chart.H>
#include<FL/Fl_Table_Row.H>
#include<FL/Fl_Tabs.H>
#include<FL/Fl_Progress.H>
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

Fl_RGB_Image* ico = new Fl_RGB_Image(idata, 32, 32, 4, 0);
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
class Scatter;
class Scoordinate;
class FreeCounter;
class FreeCounterHeader;
class Property;
class Converter;
class Cutter;
class App;
class BinPara;
class HeaderTable;
class VHeaderTable;
class MapViewWin;

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
    const int cbox_s=5,cbox_height=35;
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

class Imagesc:public Fl_Gl_Window
{
public:
    float *mat;
    float max_a,min_a,max_abs_a=0;
    float max_s,min_s,max_abs_s=0;
    bool c_norm=true,isvalid=false;
    void draw()override;
    Imagesc(int x,int y,int width,int height);
    void set_window_para();
    int handle(int event)override;
};

class Scatter:public Fl_Gl_Window
{
public:
    bool isvalid=false,ret_flag;
    char ax_buf[4][20];
    int *xs,*ys;
    int xmax,xmin,ymax,ymin,v;
    void draw()override;
    Scatter(int x,int y,int width,int height);
    ~Scatter();
    void findmvalues(int byte1,int byte2,int &max1,int &min1,int *arr1,int &max2,int &min2,int *arr2);
};

class FreeCounter:public Fl_Counter
{
    static void tc_cb(Fl_Widget *w,void *data);
    static void tc_cb_bin(Fl_Widget *w,void *data);
public:
    int step_s,step_l;
    Fl_Window *counter_set;
    Fl_Int_Input *fi_curr,*fi_step_s,*fi_step_l;
    Fl_Button *set_ok;
    FreeCounter(int x,int y,int w,int h);
    int handle(int event)override;
};

class Property:public Fl_Window
{
public:
    float fmax=FLT_MIN,fmin=FLT_MAX;
    static void global_but_cb(Fl_Widget *,void *);
    Fl_Box *text_box;
    Fl_Box *global_box;
    Fl_Button *global_but;
    char property_text[512],global_text[512];
    Property(int W,int H,const char *title);
};

class Progress:public Fl_Progress
{
public:
    Progress(int X,int Y,int W,int H);
    bool stop;
    int handle(int event)override;
};

class FreeCounterHeader:public Fl_Counter
{
public:
    Fl_Window *counter_set;
    Fl_Int_Input *fi_curr;
    Fl_Button *set_ok;
    FreeCounterHeader(int X,int Y,int W,int H,const char *title);
    int handle(int event)override;
};

class Cutter:public Fl_Window
{
    static void cut_cb(Fl_Widget *,void *);
    static void split_cb(Fl_Widget *,void *);
public:
    unsigned char hdr[3600];
    char outfilename[256],outfolder[256];
    Fl_Button *cut_but,*split_but;
    Fl_Button *open_file_cut,*open_file_split;
    Fl_Tabs *tabs;
    Fl_Group *CutGroup,*SplitGroup;
    Fl_Input *input_cut,*input_split;
    Fl_Int_Input *n_parts_input,*n_traces_input;
    Fl_Int_Input *start_trace_input,*end_trace_input,*interval_trace_input;
    Fl_Int_Input *start_sample_input,*end_sample_input,*interval_sample_input;
    Cutter(int W,int H,const char *title);
};

class Converter:public Fl_Window
{
    static void format_cb(Fl_Widget *,void *);
    static void bformat_cb(Fl_Widget *,void *);
    static void convert_cb(Fl_Widget *,void *);
    static void bconvert_cb(Fl_Widget *,void *);
public:
    bool sun2pc;
    char outfilename[256];
    char bformat=1;//0=npy,1=ieee-le
    Fl_Tabs *tabs;
    Fl_Group *fgroup,*bgroup;
    Fl_Button *convert_ok,*bconvert_ok;
    Fl_Box *notes;
    Fl_Input *input,*binput;
    Fl_Button *open_file_format,*open_file_bformat;
    Converter(int W,int H,const char *title);
};

class App:public Fl_Window
{
    int open_x=120,open_y=50;//打开文件按钮
    Fl_Text_Editor *hdr_text;
    Fl_Text_Buffer *tbuff;
    Fl_Button *open_button;
    Fl_Window *TextWin=NULL,*HdrWin=NULL,*CroppingWin=NULL,*ColorbarWin=NULL;
    Fl_Native_File_Chooser *fc,*fcc,*fcd;
    Fl_Input_Choice *colormap_c;
    static void start_cb(Fl_Widget* widget,void *);
    static void ts_cb(Fl_Widget *widget,void *);
    static void ts_cb_bin(Fl_Widget *widget,void *);
    static void hdr_cb(Fl_Widget *widget,void *);
    static void format_cb(Fl_Widget *widget,void *);
    static void enhance_cb(Fl_Widget *widget,void *);
    static void attenua_cb(Fl_Widget *widget,void *);
    static void load_clr_cb(Fl_Widget *widget,void *);
    static void save_clr_cb(Fl_Widget *widget,void *);
    static void save_sgy_data_cb(Fl_Widget *w,void *);
public:
    char mxy[64],amp_c[64];
    bool first_read=true;
    static void endian_cb(Fl_Widget *widget,void *);
    Fl_Menu_Bar *menus,*endian_menu1,*endian_menu2;
    bool is_bin=false,is_le=false;
    char filename[256],traces_c[32],samples_c[32],ld4_c[64];
    char fmt,fmk;
    unsigned char hdr[3600];
    struct manynames filenames;
    int *histo;
    FILE *fpi,*fpo,*fpd;
    unsigned long long l;
    long long traces;
    int trace_s,samples,enhance_cnt=0;
    float *seis=NULL;
    long long trace_num=0;
    const int height_of_menu=30,height_of_scroll=20;
    Fl_Chart *chart=NULL;
    const int nbin=60;
    Fl_Box *where_mouse_box=NULL,*nan_box=NULL;
    Fl_Input *input_load_hdr;
    Fl_Scrollbar *trace_slider;
    FreeCounter *trace_counter;
    App(int X,int Y,int Width,int Height,const char *title);
    ~App();
    void close_them(bool all=false);
    int handle(int event)override;
    void swap_bytes(float *seis_b,int length);
    void update_endian_menu(Fl_Menu_Bar *endian_menu);
    void detect_nan();
    void open_figure();
    void change_profile();
    void change_profile_bin();
    void format_correct(float *seis_,int length);
    void read_data(const char *fname,bool utf_flag=true);
    void ieee2ibm(float *seis_b,int length);//公式已经考虑了字节序，无需再用swap_bytes转换
};

class BinPara:public Fl_Window
{
    Fl_Box *notes;
    Fl_Button *guess,*ok;
    char notes_c[512];
public:
    bool pass=false;
    Fl_Int_Input *traces_input,*samples_input;
    BinPara(int W,int H,const char* title);
};

class HeaderTable:public Fl_Table_Row
{
    void draw_cell(TableContext context,int ROW=0,int COL=0,int X=0,int Y=0,int W=0,int H=0)override;
    void DrawHeader(const char *s,int X,int Y,int W,int H);
public:
    int trace_s_h=5,selected_R=1,bytes_num=4;
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
};

class MapViewWin:public Fl_Window
{
    Fl_Box *text;
    Fl_Button *ok;
    Fl_Int_Input *valueX;
    Fl_Int_Input *valueY;
public:
    MapViewWin(int W,int H,const char *title);
};

App *app;
BinPara *binpara;
ColorPoint **colors=NULL;
ColorPoint *tmp_c=NULL;
ColorBar *colorbar=NULL;
Imagesc *ims=NULL;
Scatter *sctr=NULL;
Property *property=NULL;
Progress *progress=NULL;
Cutter *cutter=NULL;
Converter *converter=NULL;
HeaderTable *headertable=NULL;
VHeaderTable *vht=NULL;
ResultTable *rt=NULL;
MapViewWin *mapview=NULL;
void findnames(char* filename,manynames* outfile);

#endif
