#include"headers.h"

ColorPoint::ColorPoint(int x,int y,int w,int h,double r_,double g_,double b_,ColorBar *colorbar_):Fl_Box(x,y,w,h)
{
    uchar r,g,b;
    r=r_*255;g=g_*255;b=b_*255;
    color(fl_rgb_color(r,g,b));
    box(FL_BORDER_BOX);
    colorbar=colorbar_;
}

int ColorPoint::handle(int event)
{
    int ret=Fl_Box::handle(event);
    int buttons=Fl::event_buttons()>>24;
    bool is_not_side=(idx!=0&&idx!=colorbar->c_num-1);
    switch(event)
    {
        case FL_PUSH:
        if(buttons==1&&Fl::event_clicks())
        {
            uchar r,g,b;
            Fl::get_color(color(),r,g,b);
            fl_color_chooser("choose your color",r,g,b);
            color(fl_rgb_color(r,g,b));
        }
        else if(buttons==4&&is_not_side)colorbar->remove_color_point(idx);
        ret=1;break;
        case FL_DRAG:
        if(is_not_side)
        {
            if(x()-colors[idx-1]->x()<colorbar->cbox_s*2)position(Fl::event_x()-colorbar->cbox_s,colorbar->cbox_height-12);
            else if(colors[idx+1]->x()-x()<colorbar->cbox_s*2)position(Fl::event_x()-colorbar->cbox_s,colorbar->cbox_height+12);
            else position(Fl::event_x()-colorbar->cbox_s,colorbar->cbox_height);
            colorbar->fs[idx]=(Fl::event_x()-colorbar->x())/(float)colorbar->w();
        }
        ret=1;break;
        case FL_RELEASE:
        if(is_not_side)
        {
            if(x()<=colors[idx-1]->x())position(colors[idx-1]->x()+1,colorbar->cbox_height-12);
            else if(x()>=colors[idx+1]->x())position(colors[idx+1]->x()-1,colorbar->cbox_height+12);
        }
        ret=1;
    }
    redraw();
    colorbar->parent()->redraw();
    colorbar->redraw();
    if(fwin!=nullptr)fwin->redraw();
    return ret;
}

ColorBar::ColorBar(int x,int y,int w,int h):Fl_Gl_Window(x,y,w,h)
{
    box(FL_FLAT_BOX);
    set_colormap("gray");
    sort_colors();
    end();
}

ColorBar::~ColorBar()
{
    free(fs);
    for(int i=0;i<c_num;i++)delete colors[i];
    free(colors);
}

void ColorBar::set_colormap(const char *color_idx)
{
    if(0==strcmp(color_idx,"gray"))
    {
        float gray_clrpts[]={0,0,0,0,1,1,1,1};
        set_color_points(2,gray_clrpts);
    }
    else if(0==strcmp(color_idx,"rainbow"))
        set_color_points(clr1_num,default_clr1);
    else if(0==strcmp(color_idx,"kwyr"))
        set_color_points(clr2_num,default_clr2);
    else if(0==strcmp(color_idx,"rwb"))
        set_color_points(clr3_num,default_clr3);
}

void ColorBar::set_color_points(const int clr_num,const float *defalut_clr)
{
    int i;
    for(i=0;i<c_num;i++)
    {
        parent()->remove(colors[i]);
        delete colors[i];
    }
    c_num=clr_num;
    fs=(float*)realloc(fs,c_num*sizeof(float));
    colors=(ColorPoint**)realloc(colors,c_num*sizeof(ColorPoint*));
    parent()->begin();
    for(i=0;i<c_num;i++)
    {
        fs[i]=defalut_clr[i*4];
        colors[i]=new ColorPoint(x()+w()*fs[i]-cbox_s,cbox_height,cbox_s*2,cbox_s*2,defalut_clr[i*4+1],defalut_clr[i*4+2],defalut_clr[i*4+3],this);
    }
    parent()->end();
    sort_colors();
    redraw();
    parent()->redraw();
    fwin->redraw();
}

void ColorBar::get_rgb(float f01,double &r,double &g,double &b)
{
    float t;
    int st=0;
    uchar r1,g1,b1,r2,g2,b2;
    
    while(st<c_num-1)
    {
        if(fs[st]<=f01&&f01<=fs[st+1])break;
        st++;
    }
    Fl::get_color(colors[st]->color(),r1,g1,b1);
    Fl::get_color(colors[st+1]->color(),r2,g2,b2);
    t=(f01-fs[st])/(fs[st+1]-fs[st]);
    r=(r1*(1-t)+r2*t)/255.0;
    g=(g1*(1-t)+g2*t)/255.0;
    b=(b1*(1-t)+b2*t)/255.0;
}

void ColorBar::remove_color_point(int idx)
{
    int i;
    Fl::delete_widget(colors[idx]);
    c_num--;
    for(i=idx;i<c_num;i++)
    {
        fs[i]=fs[i+1];
        colors[i]=colors[i+1];
    }
    fs=(float*)realloc(fs,c_num*sizeof(float));
    colors=(ColorPoint**)realloc(colors,c_num*sizeof(ColorPoint*));
    sort_colors();
    redraw();
    if(fwin!=nullptr)fwin->redraw();
}

void ColorBar::sort_colors()
{
    float f_t;
    ColorPoint *b_t;
    int i,j;
    for(i=0;i<c_num-1;i++)
    {
        for(j=0;j<c_num-i-1;j++)
        {
            if(fs[j]>fs[j+1])
            {
                f_t=fs[j];
                fs[j]=fs[j+1];
                fs[j+1]=f_t;
                b_t=colors[j];
                colors[j]=colors[j+1];
                colors[j+1]=b_t;
            }
        }
    }
    for(i=0;i<c_num;i++)colors[i]->idx=i;
}

void ColorBar::draw()
{
    int i,c;
    double r,g,b;
    glViewport(0,0,w(),h());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();//重置
    glOrtho(0,w(),h(),0,-1,1);
    glClearColor(1,1,1,0);
    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_QUADS);
    for(i=0;i<w();i++)
    {
        get_rgb(i/(float)w(),r,g,b);
        glColor3f(r,g,b);
        glVertex2i(i,0);
        glVertex2i(i,h());
        glVertex2i(i+1,h());
        glVertex2i(i+1,0);
    }
    glEnd();
}

int ColorBar::handle(int event)
{
    double r,g,b;
    float fidx;
    int ret=Fl_Gl_Window::handle(event);
    switch(event)
    {
        case FL_PUSH:
        colors=(ColorPoint**)realloc(colors,(c_num+1)*sizeof(ColorPoint*));
        fs=(float*)realloc(fs,(c_num+1)*sizeof(float));
        fidx=(Fl::event_x())/(float)w();
        get_rgb(fidx,r,g,b);
        parent()->begin();
        colors[c_num]=new ColorPoint(Fl::event_x()+x()-cbox_s,cbox_height,cbox_s*2,cbox_s*2,r,g,b,this);
        parent()->end();

        fs[c_num]=fidx;
        c_num++;
        //排序色标
        sort_colors();
        parent()->redraw();
        return 1;
    }
    return ret;
}

Imagesc::Imagesc(int x,int y,int width,int height):Fl_Gl_Window(x,y,width,height){}
void Imagesc::draw()
{
    if(!set_mat)return;
    int i,j;
    glViewport(0,0,parent()->w(),parent()->h());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();//重置
    glOrtho(0,rows,cols,0,-1,1);
    glClearColor(1,1,1,0);
    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_QUADS);

    if(!centered_norm)
    {
        max_a=mat[0];
        min_a=mat[0];
        for(i=0;i<rows*cols;i++)
        {
            if(max_a<mat[i])max_a=mat[i];
            if(min_a>mat[i])min_a=mat[i];
        }
        for(i=0;i<rows;i++)
        {
            for(j=0;j<cols;j++)
            {
                float normdata=(mat[i*cols+j]-min_a)/(max_a-min_a);
                double r,g,b;
                colorbar->get_rgb(normdata,r,g,b);
                glColor3f(r,g,b);
                glVertex2i(i,j);
                glVertex2i(i+1,j);
                glVertex2i(i+1,j+1);
                glVertex2i(i,j+1);
            }
        }
    }
    else
    {
        max_abs_a=0;
        for(i=0;i<rows*cols;i++)
            if(max_abs_a<abs(mat[i]))max_abs_a=abs(mat[i]);
        for(i=0;i<rows;i++)
        {
            for(j=0;j<cols;j++)
            {
                float normdata=0.5*(mat[i*cols+j]/max_abs_a+1);
                double r,g,b;
                colorbar->get_rgb(normdata,r,g,b);
                glColor3f(r,g,b);
                glVertex2i(i,j);
                glVertex2i(i+1,j);
                glVertex2i(i+1,j+1);
                glVertex2i(i,j+1);
            }
        }
    }
    glEnd();   
}

void Imagesc::set_window_para(float *mat_f,int rows_f,int cols_f)
{
    mat=mat_f;
    rows=rows_f;
    cols=cols_f;
    set_mat=true;
    redraw();
}

void Free_Counter::tc_cb(Fl_Widget *w,void *data)
{
    Fl_Counter *fct=(Fl_Counter*)w;
    long long V=(long long)fct->value();
    app->trace_num=V;
    app->change_profile();
    app->trace_slider->value(V);
}

void Free_Counter::ok_cb(Fl_Widget *w,void *data)
{
    Free_Counter *frc=(Free_Counter*)data;
    long long V=atoi(frc->curr->value());
    app->trace_num=V;
    frc->value((double)V);
    app->trace_slider->value((double)V);
    frc->small_step=atoi(frc->small_step_input->value());
    frc->large_step=atoi(frc->large_step_input->value());
    frc->step(frc->small_step,frc->large_step);
    app->change_profile();
    Fl::delete_widget(frc->counter_set);
}

Free_Counter::Free_Counter(int x,int y,int w,int h):Fl_Counter(x,y,w,h)
{
    small_step=1;
    large_step=app->w();
    bounds(0,app->traces-app->trace_s);
    step(small_step,large_step);
    callback(tc_cb);
}

int Free_Counter::handle(int event)
{
    int ret=Fl_Counter::handle(event);
    switch(event)
    {
        case FL_PUSH:
        if(x()+w()*0.3<Fl::event_x()&&Fl::event_x()<x()+w()*0.7)
        {
            char small_step_c[10],large_step_c[10],curr_c[20];
            itoa(small_step,small_step_c,10);
            itoa(large_step,large_step_c,10);
            itoa(app->trace_num,curr_c,10);
            
            counter_set=new Fl_Window(300,150,"请输入你想输入的参数喵");
            counter_set->set_modal();
            counter_set->begin();
            curr=new Fl_Int_Input(80,10,100,30,"current");
            curr->value(curr_c);
            small_step_input=new Fl_Int_Input(80,50,100,30,"small step");
            small_step_input->value(small_step_c);
            large_step_input=new Fl_Int_Input(80,90,100,30,"large step");
            large_step_input->value(large_step_c);
            set_ok=new Fl_Button(200,50,80,30,"确认");
            set_ok->callback(ok_cb,this);
            counter_set->end();
            counter_set->show();
        }
        ret=1;
        break;
    }
    return ret;
}

void Free_Counter_Header::tc_cb(Fl_Widget *w,void *data)
{
    Fl_Counter *fc=(Fl_Counter*)w;
    tab->trace_num=(long long)fc->value();
    tab->get_hdr();
}

Free_Counter_Header::Free_Counter_Header(int X,int Y,int W,int H,const char *title):Fl_Counter(X,Y,W,H,title)
{
    type(FL_SIMPLE_COUNTER);
    step(1);
    bounds(0,app->traces-tab->trace_s_h);
    align(FL_ALIGN_TOP);
    callback(tc_cb);
};

int Free_Counter_Header::handle(int event)
{
    int ret=Fl_Counter::handle(event);
    switch(event)
    {
        case FL_PUSH:
        if(x()+w()*3/16<Fl::event_x()&&Fl::event_x()<x()+w()*13/16)
        {
            char curr_c[20];
            itoa(tab->trace_num,curr_c,10);
            counter_set=new Fl_Window(300,150,"请输入你想输入的道数喵");
            counter_set->set_modal();
            counter_set->begin();
            curr=new Fl_Int_Input(80,10,100,30,"current");
            curr->value(curr_c);
            set_ok=new Fl_Button(200,50,80,30,"确认");
            set_ok->callback([](Fl_Widget *,void *data)
            {
                Free_Counter_Header *fch=(Free_Counter_Header*)data;
                long long V=atoi(fch->curr->value());
                tab->trace_num=V;
                Fl::delete_widget(fch->counter_set);
                tab->get_hdr();
            },this);
            counter_set->end();
            counter_set->show();
        }
        ret=1;
        break;
    }
    return ret;
}

void App::start_cb(Fl_Widget *widget,void *)
{
    char err;
    app->fc->title("Open SEGY File");
    app->fc->type(Fl_Native_File_Chooser::BROWSE_FILE);
    switch(app->fc->show())
    {
        case -1:break;//Error
        case 1:break;//Cancel
        default:
        app->read_data(app->fc->filename());
        break;
    }
}

void App::ts_cb(Fl_Widget *widget,void *)
{
    Fl_Value_Slider *fcs=(Fl_Value_Slider*)widget;
    long long V=(long long)fcs->value();
    app->trace_num=V;
    app->change_profile();
    app->trace_counter->value(V);
}

void App::change_profile()
{
    fseeko64(fpi,3600+trace_num*(240+4*samples),0);
    for(int i=0;i<trace_s;i++)
    {
        fseeko64(fpi,240,1);
        fread(seis+samples*i,4,samples,fpi);
    }
    if(!is_pc&&!is_le)ieee2ibm();
    else if(is_pc&&!is_le)swap_bytes();
    else if(!is_pc&&is_le)
    {
        ieee2ibm();
        swap_bytes();
    }
    open_figure();
}

void App::swap_bytes()
{
    unsigned char *p,tmp;
    for(long long tr=0;tr<trace_s*samples;tr++)
    {
        p=(unsigned char*)&seis[tr];
        p[0]=tmp,p[0]=p[3];p[3]=tmp;
        p[1]=tmp,p[1]=p[2];p[2]=tmp;
    }
}

void App::save_cb(Fl_Widget *widget,void *)
{
    if(app->SaveWin==NULL){app->SaveWin=new Fl_Window(400,200,"Save");app->PropertyWin->icon(ico);}
    else{app->SaveWin->show();return;}//子窗口关闭后仅隐藏
    app->SaveWin->show();
}

void App::property_cb(Fl_Widget *widget,void *)
{
    if(app->PropertyWin==NULL){app->PropertyWin=new Fl_Window(500,500,"property");app->PropertyWin->icon(ico);}
    else {app->PropertyWin->show();return;}//防止老六开很多窗口

    char property_text[512];
    sprintf(property_text,"traces:%lld\nsamples:%d\n",app->traces,app->samples);
    app->PropertyWin->begin();
    Fl_Box *text_box=new Fl_Box(10,10,480,480);
    text_box->copy_label(property_text);
    text_box->box(FL_FLAT_BOX);
    app->PropertyWin->end();
    app->PropertyWin->show();
}

void App::hdr_cb(Fl_Widget *widget,void *)
{
    if(app->HdrWin==NULL){app->HdrWin=new Fl_Window(600,600,"Header Text");app->HdrWin->icon(ico);}
    else {app->HdrWin->show();return;}
    app->HdrWin->begin();
    {
        tab=new Header_Table(20,20,460,440);
        Free_Counter_Header *fch=new Free_Counter_Header(20,550,160,30,"Trace:");
    }
    app->HdrWin->end();
    app->HdrWin->show();
}

void App::cutter_separater_cb(Fl_Widget *widget,void *)
{
    if(app->CroppingWin==NULL)app->CroppingWin=new Fl_Window(400,200,"Cropping");
    else {app->CroppingWin->show();return;}

    app->CroppingWin->show();
}

void App::format_cb(Fl_Widget *widget,void *)
{
    Fl_Menu_Bar *bar = (Fl_Menu_Bar*)widget;
    const Fl_Menu_Item *item = bar->mvalue();
    char ipath[64];
    bar->item_pathname(ipath,sizeof(ipath));
    if(0==strcmp(item->label(),"ibm-le"))
    {
        app->is_le=true;
        app->is_pc=false;
    }
    else if(0==strcmp(item->label(),"ibm-be"))
    {
        app->is_le=false;
        app->is_pc=false;
    }
    else if(0==strcmp(item->label(),"ieee-le"))
    {
        app->is_le=true;
        app->is_pc=true;
    }
    else if(0==strcmp(item->label(),"ieee-be"))
    {
        app->is_le=false;
        app->is_pc=true;
    }
    app->change_profile();
}

void App::enhance_cb(Fl_Widget *widget,void *)
{
    app->enhance_cnt++;
    for(int i=0;i<app->trace_s*app->samples;i++)
        app->seis[i]=app->seis[i]<0?-log(1-app->seis[i]):log(1+app->seis[i]);
    fwin->redraw();
}

void App::attenua_cb(Fl_Widget *widget,void *)
{
    if(app->enhance_cnt==0)return;
    app->enhance_cnt--;
    for(int i=0;i<app->trace_s*app->samples;i++)
        app->seis[i]=app->seis[i]<0?1-exp(-app->seis[i]):exp(app->seis[i])-1;
    fwin->redraw();
}

void App::load_clr_cb(Fl_Widget *w,void *)
{
    app->fcc->title("Load colormap");
    app->fcc->type(Fl_Native_File_Chooser::BROWSE_FILE);
    switch(app->fcc->show())
    {
        case -1:break;
        case 1:break;
        default:
        FILE *fpc;
        int find_c_num;
        fpc=fl_fopen(app->fcc->filename(),"r");
        fscanf(fpc,"%d",&find_c_num);
        float *custom_color=(float*)malloc(16*colorbar->c_num);
        for(int i=0;i<colorbar->c_num*4;i++)
            fscanf(fpc,"%f,",&custom_color[i]);

        colorbar->set_color_points(find_c_num,custom_color);
        colorbar->sort_colors();
        colorbar->redraw();
        app->ClrbWin->redraw();
        fwin->redraw();
        fclose(fpc);
        free(custom_color);
    }
}

void App::save_clr_cb(Fl_Widget *w,void *)
{
    app->fcc->title("Save colormap as");
    app->fcc->type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    switch(app->fcc->show())
    {
        case -1:break;
        case 1:break;
        default:
        FILE *fpc;
        fpc=fl_fopen(app->fcc->filename(),"w");
        fprintf(fpc,"%d\n",colorbar->c_num);
        for(int i=0;i<colorbar->c_num;i++)
        {
            uchar r,g,b;
            Fl::get_color(colors[i]->color(),r,g,b);
        };
        fclose(fpc);
    }
}

void App::save_sgy_data_cb(Fl_Widget *w,void *)
{
    FILE *fpo;
    int i,j;
    app->fc->title("Save As");
    app->fc->type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    switch(app->fc->show())
    {
        case -1: break;
        case 1: break;
        default:
        fpo=fl_fopen(app->fc->filename(),"wb");
        char asc_out[3200]={32};
        int tb_len=app->hdr_text->buffer()->length();
        unsigned char ebc_out[40][80]={0};
        for(i=0;i<(tb_len>3200)?3200:tb_len;i++)
            asc_out[i]=app->tbuff->text()[i];

        for(i=0;i<40;i++)
        {
            for(j=0;j<80;j++)
                ebc_out[i][j]=a2e[asc_out[i*80+j]];
            fwrite(ebc_out[i],1,80,fpo);
        }
        fwrite(app->hdr+3200,1,400,fpo);
        fseeko64(app->fpi,3600,0);
        unsigned char *tbuf=(unsigned char*)malloc(240+4*app->samples);
        for(long long tr=0;tr<app->traces;tr++)
        {
            fread(tbuf,1,240+4*app->samples,app->fpi);
            fwrite(tbuf,1,240+4*app->samples,fpo);
        }
        free(tbuf);
        fclose(fpo);
    }
}

App::App(int X,int Y,int Width,int Height,const char *title):Fl_Window(X,Y,Width,Height,title)
{
    Fl::scheme("gtk+");

    icon(ico);
    begin();
    fc=new Fl_Native_File_Chooser();
    fcc=new Fl_Native_File_Chooser();
    fc->filter("SEGY\t*.sgy;*.segy;*.SEGY;*.segy");
    fcc->filter("Text\t*.txt;*.csv;*.clr");
    menus=new Fl_Menu_Bar(0,0,Width,height_of_menu);
    menus->add("File/Save as",0,save_cb,0,FL_MENU_INACTIVE);
    menus->add("File/Property",0,property_cb,0,FL_MENU_INACTIVE);
    menus->add("Tools/Textual Header Editor",0,[](Fl_Widget *,void *){app->TextWin->show();},0,FL_MENU_INACTIVE);
    menus->add("Tools/Headers",0,hdr_cb,0,FL_MENU_INACTIVE);
    menus->add("Tools/Cutter-Separater",0,cutter_separater_cb,0,FL_MENU_INACTIVE);
    menus->add("Tools/色标",0,[](Fl_Widget *w,void *){app->ClrbWin->show();},0,FL_MENU_DIVIDER);
    menus->add("Tools/图像增强","^q",enhance_cb,0,FL_MENU_INACTIVE);
    menus->add("Tools/图像减弱","^w",attenua_cb,0,FL_MENU_INACTIVE);
    menus->add("data?/ibm-le",0,format_cb,0,FL_MENU_INACTIVE|FL_MENU_RADIO);
    menus->add("data?/ibm-be",0,format_cb,0,FL_MENU_INACTIVE|FL_MENU_RADIO);
    menus->add("data?/ieee-le",0,format_cb,0,FL_MENU_INACTIVE|FL_MENU_RADIO);
    menus->add("data?/ieee-be",0,format_cb,0,FL_MENU_INACTIVE|FL_MENU_RADIO|FL_MENU_DIVIDER);
    menus->add("data?/zero-centered",0,[](Fl_Widget *w,void *)
    {
        Fl_Menu_Bar *bar = (Fl_Menu_Bar*)w;
        const Fl_Menu_Item *item = bar->mvalue();
        fwin->centered_norm=item->value()?true:false;
        fwin->redraw();
    },this,FL_MENU_INACTIVE|FL_MENU_TOGGLE); 
    
    fwin=new Imagesc(0,height_of_menu,w(),h()-height_of_menu);
    open_button=new Fl_Button((Width-open_x)/2,(Height-open_y)/2,open_x,open_y);
    fwin->hide();
    open_button->label("打开文件\n或将文件拖入窗口");
    open_button->callback(start_cb);
    resizable(*fwin);
    end();

    ClrbWin=new Fl_Window(660,300,"Colorbar");
    ClrbWin->icon(ico);
    ClrbWin->begin();
    {
        const int cheight2=100,cheight3=200;
        Fl_Box *explaination=new Fl_Box(200,150,400,140);
        explaination->box(FL_FLAT_BOX);
        explaination->align(FL_ALIGN_TOP|FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
        explaination->label("双击色标点以更改颜色\n\n输入数据格式:\n第一行为色标点个数\n第二行开始为0-1之间的4列实数\n第一列为色标值,后三列为r,g,b值\n以逗号隔开");
        // explaination->color(FL_RED);
        Fl_Button *swap_clr=new Fl_Button(50,cheight2,100,30,"reverse");
        Fl_Button *load_clr=new Fl_Button(50,cheight3,100,30,"load colormap");
        load_clr->callback(load_clr_cb);
        Fl_Button *save_clr=new Fl_Button(50,cheight3+40,100,30,"save colormap");
        save_clr->callback(save_clr_cb);
        colormap_c=new Fl_Input_Choice(500,cheight2,100,30,"Colormap:");
        colormap_c->add("gray");
        colormap_c->add("rainbow");
        colormap_c->add("kwyr");
        colormap_c->add("rwb");
        colormap_c->value(0);
        colormap_c->callback([](Fl_Widget *w, void *)
        {
            Fl_Input_Choice *cmp=(Fl_Input_Choice*)w;
            char color_idx[16];
            strcpy(color_idx,cmp->value());
            colorbar->set_colormap(color_idx);
        });
        colorbar=new ColorBar(30,60,600,20);
        swap_clr->callback([](Fl_Widget*, void *)
        {
            float tmp_f;
            int tmp_x,i;
            for(i=0;i<colorbar->c_num/2;i++)
            {
                tmp_c=colors[i];
                colors[i]=colors[colorbar->c_num-i-1];
                colors[colorbar->c_num-i-1]=tmp_c;

                tmp_x=colors[i]->x();
                colors[i]->position(colors[colorbar->c_num-i-1]->x(),colors[i]->y());
                colors[colorbar->c_num-i-1]->position(tmp_x,colors[colorbar->c_num-i-1]->y());
            }
            for(i=0;i<colorbar->c_num;i++)colors[i]->redraw();
            colorbar->redraw();
            fwin->redraw();
        });
    }
    ClrbWin->redraw();
    ClrbWin->end();
    ClrbWin->hide();
}

App::~App()
{
    fclose(fpi);
    free(seis);
}

int App::handle(int event)
{
    int ret=Fl_Window::handle(event);
    switch(event)
    {
        case FL_MOVE:
        if(!first_read)
        {
            sprintf(mxy,"X:%d\tY:%d",trace_s*Fl::event_x()/w(),samples*Fl::event_y()/h());
            where_mouse_box->copy_label(mxy);
        }
        case FL_DND_ENTER:
        case FL_DND_DRAG:
        case FL_DND_RELEASE:ret=1;break;
        case FL_PASTE:
        char err;
        read_data(Fl::event_text());
        break;
    }
    return ret;
};

void App::open_figure()
{
    fwin->set_window_para(seis,trace_s,samples);
    fwin->show();
    for(int cnt=0;cnt<enhance_cnt;cnt++)
        for(int i=0;i<trace_s*samples;i++)
            seis[i]=seis[i]<0?-log(1-seis[i]):log(1+seis[i]);
    redraw();
    if(first_read)
    {
        begin();
        where_mouse_box=new Fl_Box(150,0,100,height_of_menu);
        end();
        //ebcdic
        TextWin=new Fl_Window(740,820,"Header Text");
        TextWin->begin();
        {
            TextWin->icon(ico);
            hdr_text=new Fl_Text_Editor(10,10,720,720);
            tbuff=new Fl_Text_Buffer();
            hdr_text->buffer(tbuff);
            unsigned char ebc[40][80];
            char asc[40][80];
            int i,j;
            fseeko64(fpi,0,0);
            for(i=0;i<40;i++)
            {
                fread(ebc[i],1,80,fpi);
                for(j=0;j<80;j++)
                    asc[i][j]=e2a[ebc[i][j]];
                asc[i][79]='\0';
                tbuff->append(asc[i]);
                tbuff->append("\n");
            }
            Fl_Button *save_but=new Fl_Button(300,750,100,30,"save as");
            save_but->callback(save_sgy_data_cb);
        }
        TextWin->end();
        TextWin->hide();

        open_button->hide();
        Fl_Menu_Item *item=(Fl_Menu_Item*)menus->find_item("Tools/Cutter-Separater");item->flags=0;//激活按钮
        item=(Fl_Menu_Item*)menus->find_item("Tools/Headers");item->flags=0;
        item=(Fl_Menu_Item*)menus->find_item("Tools/Textual Header Editor");item->flags=0;
        item=(Fl_Menu_Item*)menus->find_item("Tools/图像增强");item->flags=0;
        item=(Fl_Menu_Item*)menus->find_item("Tools/图像减弱");item->flags=0;
        item=(Fl_Menu_Item*)menus->find_item("File/Save as");item->flags=0;
        item=(Fl_Menu_Item*)menus->find_item("File/Property");item->flags=0;
        item=(Fl_Menu_Item*)menus->find_item("data?/ibm-le");item->flags=FL_MENU_RADIO;
        item=(Fl_Menu_Item*)menus->find_item("data?/ibm-be");item->flags=FL_MENU_RADIO;
        item=(Fl_Menu_Item*)menus->find_item("data?/ieee-le");item->flags=FL_MENU_RADIO;
        item=(Fl_Menu_Item*)menus->find_item("data?/ieee-be");item->flags=FL_MENU_RADIO|FL_MENU_DIVIDER;
        item=(Fl_Menu_Item*)menus->find_item("data?/zero-centered");item->flags=FL_MENU_TOGGLE;

        if(is_pc&&is_le){item=(Fl_Menu_Item*)menus->find_item("data?/ieee-le");item->set();}
        else if(is_pc&&!is_le){item=(Fl_Menu_Item*)menus->find_item("data?/ieee-be");item->set();}
        else if(!is_pc&&is_le){item=(Fl_Menu_Item*)menus->find_item("data?/ibm-le");item->set();}
        else if(!is_pc&&!is_le){item=(Fl_Menu_Item*)menus->find_item("data?/ibm-be");item->set();}
        if(traces>w())//文件太大则制作slider
        {
            begin();
            trace_slider=new Fl_Value_Slider(w()-350,0,150,height_of_menu);
            trace_counter=new Free_Counter(w()-200,0,200,height_of_menu);
            end();
            trace_slider->align(FL_ALIGN_LEFT);
            trace_slider->bounds(0,traces-trace_s);
            trace_slider->type(1);
            trace_slider->step(1);
            trace_slider->callback(ts_cb,this);
        }
        first_read=false;
    }
}

void App::read_data(const char *fname)
{
    int i,j;
    
    fpi=fl_fopen(fname,"rb");
    if(fpi==NULL)
    {
        fl_alert("文件不存在！");
        return;
    }
    unsigned long long l;
    if(seis!=NULL)
    {
        free(seis);
        seis=NULL;
    }
    fseeko64(fpi,0,SEEK_END);
    l=ftello64(fpi);
    fseeko64(fpi,0,0);
    fread(hdr,1,3600,fpi);
    if(hdr[3224]+hdr[3225]!=1&&hdr[3224]+hdr[3225]!=5)
    {
        return; //
    }
    is_pc=(hdr[3224]+hdr[3225]==1)?false:true;
    if(hdr[3224]==0)
    {
        is_le=false;
        samples=hdr[3221]+hdr[3220]*256;
    }
    else if(hdr[3225]==0)
    {
        is_le=true;
        samples=hdr[3221]*256+hdr[3220];
    }
    // fprintf(stderr,"The file format is:%d,%d\n",hdr[3224],hdr[3225]);
    // fprintf(stderr,"samples=%d\n",samples);
    traces=(l-3600)/(long long)(240+4*samples);
    trace_s=(traces<w())?traces:w();
    fseeko64(fpi,3600,0);
    seis=(float*)malloc(sizeof(float)*trace_s*samples);
    for(i=0;i<trace_s;i++)
    {
        fseeko64(fpi,240,1);
        fread(seis+samples*i,4,samples,fpi);
    }//
    if(!is_pc&&!is_le)ieee2ibm();
    else if(is_pc&&!is_le)swap_bytes();
    else if(!is_pc&&is_le)
    {
        ieee2ibm();
        swap_bytes();
    }
    trace_num=0;
    open_figure();
    strcpy(filename,fname);
    label(fname);
}

void App::ieee2ibm()//公式已经考虑了字节序，无需再用swap_bytes转换
{
    char sgn;
    unsigned char E,*p;
    for(long long tr=0;tr<trace_s*samples;tr++)
    {
        if((seis[tr])!=0)
        {
            p=(unsigned char*)&seis[tr];
            sgn=1-2*(p[0]>>7);
            E=p[0]<<1;
            E>>=1;
            seis[tr]=sgn*((*(p+1)<<16)+(*(p+2)<<8)+*(p+3))*pow(16,(int)E-70);
        }
    }
}

void Header_Table::draw_cell(TableContext context, int ROW, int COL, int X, int Y, int W, int H)
{
    static char s[32];
    switch(context)
    {
    case CONTEXT_STARTPAGE:
        fl_font(FL_HELVETICA, 16);
        return;
    case CONTEXT_COL_HEADER:
        sprintf(s,"trace%d",COL+1);
        DrawHeader(s,X,Y,W,H);
        return;
    case CONTEXT_ROW_HEADER:
        sprintf(s,"b%d",ROW+1);
        DrawHeader(s,X,Y,W,H);
        return;
    case CONTEXT_CELL:
        sprintf(s,"%d",hdrbytes_s[COL*240+ROW]);
        fl_push_clip(X,Y,W,H);
        fl_draw_box(FL_THIN_UP_BOX,X,Y,W,H,row_selected(ROW)?FL_YELLOW:FL_WHITE);
        fl_color(FL_GRAY0);fl_draw(s,X,Y,W,H,FL_ALIGN_CENTER);
        fl_pop_clip();
        return;
    default:
        return;
    }
}

void Header_Table::DrawHeader(const char *s, int X, int Y, int W, int H)
{
    fl_push_clip(X,Y,W,H);
    fl_draw_box(FL_THIN_UP_BOX,X,Y,W,H,row_header_color());
    fl_color(FL_BLACK);
    fl_draw(s,X,Y,W,H,FL_ALIGN_CENTER);
    fl_pop_clip();
}

Header_Table::Header_Table(int X,int Y,int W,int H):Fl_Table_Row(X,Y,W,H)
{
    hdrbytes_s=(unsigned char*)malloc(trace_s_h*240);
    rows(240);
    row_height_all(20);
    row_header(1);
    cols(trace_s_h);
    col_width_all(80);
    col_header(1);
    end();
    get_hdr();
}

void Header_Table::get_hdr()
{
    fseeko64(app->fpi,3600+trace_num*(240+4*app->samples),0);
    for(int i=0;i<trace_s_h;i++)
    {
        fread(hdrbytes_s+i*240,1,240,app->fpi);
        fseeko64(app->fpi,app->samples*4,1);
    }
    redraw();
}

Header_Table::~Header_Table()
{
    free(hdrbytes_s);
}

int main(int argc, char **argv)
{
    app=new App(100,100,600,900,"Open a segy file");
    app->show(1,argv);
    if(argc>1)
        app->read_data(argv[1]);
    return Fl::run();
}