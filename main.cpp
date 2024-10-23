#include"headers.h"
//已知bug:直接将文件拖入.exe文件以直接打开sgy需要没有中文路径，原因在于fl_fopen(filename,"rb")中的filename不是utf-8
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
    // fprintf(stderr,"draw!\n");
    if(!set_mat)return;
    int i,j;
    glViewport(0,0,parent()->w(),parent()->h());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();//重置
    glOrtho(0,rows,cols,0,-1,1);
    glClearColor(1,1,1,0);
    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_QUADS);
    
    double r,g,b;
    if(!centered_norm)
    {
        float rr;
        rr=max_a-min_a;
        if(rr>FLT_MAX)
        {
            colorbar->get_rgb(0,r,g,b);
            glColor3f(r,g,b);
            glVertex2i(0,0);glVertex2i(rows,0);glVertex2i(rows,cols);glVertex2i(0,cols);glEnd();
            return;
        }
        for(i=0;i<rows;i++)
        {
            for(j=0;j<cols;j++)
            {
                float normdata=(mat[i*cols+j]-min_a)/(rr+FLT_MIN);
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
        for(i=0;i<rows;i++)
        {
            for(j=0;j<cols;j++)
            {
                float normdata=0.5*(mat[i*cols+j]/(max_abs_a+FLT_MIN)+1);
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
    if(!centered_norm)
    {
        max_a=mat[0];
        min_a=mat[0];
        for(int i=0;i<rows*cols;i++)
        {
            if(max_a<mat[i])max_a=mat[i];
            if(min_a>mat[i])min_a=mat[i];
        }
    }
    else
    {
        max_abs_a=0;
        for(int i=0;i<rows*cols;i++)
            if(max_abs_a<fabs(mat[i]))max_abs_a=fabs(mat[i]);
    }
    set_mat=true;
}

int Imagesc::handle(int event)
{
    int ret=Fl_Gl_Window::handle(event);
    switch(event)
    {
        case FL_DND_ENTER:
        case FL_DND_DRAG:
        case FL_DND_RELEASE:ret=1;break;
        case FL_PASTE:
        app->read_data(Fl::event_text());
    }
    return ret;
}

void FreeCounter::tc_cb(Fl_Widget *w,void *data)
{
    Fl_Counter *fct=(Fl_Counter*)w;
    long long V=(long long)fct->value()-1;
    app->trace_num=V;
    app->change_profile();
    app->trace_slider->value(V+1);
}

void FreeCounter::ok_cb(Fl_Widget *w,void *data)
{
    FreeCounter *frc=(FreeCounter*)data;
    long long V=atoi(frc->fi_curr->value());
    app->trace_num=V;
    frc->value((double)V);
    app->trace_slider->value((double)V);
    frc->step_s=atoi(frc->fi_step_s->value());
    frc->step_l=atoi(frc->fi_step_l->value());
    frc->step(frc->step_s,frc->step_l);
    app->change_profile();
    Fl::delete_widget(frc->counter_set);
}

FreeCounter::FreeCounter(int x,int y,int w,int h):Fl_Counter(x,y,w,h)
{
    step_s=1;
    step_l=app->w();
    bounds(1,app->traces-app->trace_s);
    step(step_s,step_l);
    callback(tc_cb);
}

int FreeCounter::handle(int event)
{
    int ret=Fl_Counter::handle(event);
    switch(event)
    {
        case FL_PUSH:
        if(x()+w()*0.3<Fl::event_x()&&Fl::event_x()<x()+w()*0.7)
        {
            char cstep_s[10],cstep_l[10],ccurr[20];
            sprintf(cstep_s,"%d",step_s);
            sprintf(cstep_l,"%d",step_l);
            sprintf(ccurr,"%lld",app->trace_num);
            
            counter_set=new Fl_Window(300,150,"请输入你想输入的参数喵");
            counter_set->set_modal();
            counter_set->begin();
            fi_curr=new Fl_Int_Input(80,10,100,30,"current");
            fi_curr->value(ccurr);
            fi_step_s=new Fl_Int_Input(80,50,100,30,"small step");
            fi_step_s->value(cstep_s);
            fi_step_l=new Fl_Int_Input(80,90,100,30,"large step");
            fi_step_l->value(cstep_l);
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

void FreeCounterHeader::tc_cb(Fl_Widget *w,void *data)
{
    Fl_Counter *fc=(Fl_Counter*)w;
    headertable->trace_num=(long long)fc->value()-1;
    headertable->get_hdr();
    headertable->redraw();
    rt->redraw();
}

FreeCounterHeader::FreeCounterHeader(int X,int Y,int W,int H,const char *title):Fl_Counter(X,Y,W,H,title)
{
    type(FL_SIMPLE_COUNTER);
    step(1);
    bounds(1,app->traces-headertable->trace_s_h+1);
    value(1);
    align(FL_ALIGN_TOP);
    callback(tc_cb);
};

int FreeCounterHeader::handle(int event)
{
    int ret=Fl_Counter::handle(event);
    switch(event)
    {
        case FL_PUSH:
        if(x()+w()*3/16<Fl::event_x()&&Fl::event_x()<x()+w()*13/16)
        {
            char ccurr[32],trace_range_s[64];
            sprintf(ccurr,"%lld",headertable->trace_num+1);
            sprintf(trace_range_s,"current:(1-%lld)",app->traces-headertable->trace_s_h+1);
            counter_set=new Fl_Window(300,150,"请输入你想输入的道数喵");
            counter_set->set_modal();
            counter_set->begin();
            fi_curr=new Fl_Int_Input(120,10,100,30);
            fi_curr->copy_label(trace_range_s);
            fi_curr->value(ccurr);
            set_ok=new Fl_Button(200,50,80,30,"确认");
            set_ok->callback([](Fl_Widget *,void *data)
            {
                FreeCounterHeader *fch=(FreeCounterHeader*)data;
                long long V=atoi(fch->fi_curr->value());
                headertable->trace_num=V-1;
                fch->value(V);
                Fl::delete_widget(fch->counter_set);
                headertable->get_hdr();
                headertable->redraw();
                rt->redraw();
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
    long long V=(long long)fcs->value()-1;
    app->trace_num=V;
    app->change_profile();
    app->trace_counter->value(V+1);
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
    unsigned char *p,tmp_c;
    for(long long tr=0;tr<trace_s*samples;tr++)
    {
        p=(unsigned char*)&seis[tr];
        tmp_c=p[0],p[0]=p[3];p[3]=tmp_c;
        tmp_c=p[1],p[1]=p[2];p[2]=tmp_c;
    }
}

void App::save_cb(Fl_Widget *widget,void *)
{
    if(app->SaveWin==NULL){app->SaveWin=new Fl_Window(400,200,"Save");app->SaveWin->icon(ico);}
    else{app->SaveWin->show();return;}//子窗口关闭后仅隐藏
    app->SaveWin->show();
}

void App::property_cb(Fl_Widget *widget,void *)
{
    if(app->PropertyWin==NULL){app->PropertyWin=new Fl_Window(500,500,"property");app->PropertyWin->icon(ico);}
    else {app->PropertyWin->show();return;}//防止开很多窗口

    char property_text[512];
    float dt;
    dt=(app->hdr[3224]==0)?(app->hdr[3216]*256+app->hdr[3217]):(app->hdr[3216]+app->hdr[3217]*256);
    dt/=1000;
    sprintf(property_text,"traces:%lld\nsamples:%d\ndt:%g ms\n",app->traces,app->samples,dt);
    app->PropertyWin->begin();
    Fl_Box *text_box=new Fl_Box(10,10,480,480);
    text_box->copy_label(property_text);
    text_box->box(FL_FLAT_BOX);
    app->PropertyWin->end();
    app->PropertyWin->show();
}

void App::hdr_cb(Fl_Widget *widget,void *)
{
    if(app->HdrWin==NULL){app->HdrWin=new Fl_Window(660,600,"Headers");app->HdrWin->icon(ico);}
    else {app->HdrWin->show();app->HdrWin->icon(ico);return;}
    app->HdrWin->begin();
    {
        headertable=new HeaderTable(20,20,460,440,"Trace Header");
        rt=new ResultTable(20,480,442,22,0);
        vht=new VHeaderTable(500,20,140,560,"Volume Header");
        FreeCounterHeader *fch=new FreeCounterHeader(20,520,160,30,"Trace:");
        Fl_Menu_Bar *endian_menu=new Fl_Menu_Bar(200,520,200,30);
        endian_menu->add("big-endian",0,endian_cb,0,FL_MENU_RADIO);
        endian_menu->add("little-endian",0,endian_cb,0,FL_MENU_RADIO);
        Fl_Menu_Item *item=(Fl_Menu_Item*)endian_menu->find_item("big-endian");
        item->set(); //默认为big-endian
        Fl_Button *export_hdr=new Fl_Button(20,560,100,30,"export value");
        Fl_Button *load_hdr=new Fl_Button(130,560,100,30,"load value");
        app->save_hdr=new Fl_Button(240,560,100,30,"save!");
        app->save_hdr->deactivate();
        export_hdr->callback([](Fl_Widget *,void *)
        {
            int hdrv;
            unsigned char *p=(unsigned char*)&hdrv;
            FILE *fp;
            fp=fl_fopen("output.txt","w");
            fseeko64(app->fpi,3600,0);
            for(long long tr=0;tr<app->traces;tr++)
            {
                fseeko64(app->fpi,headertable->selected_R-1,1);
                if(rt->is_le){p[0]=fgetc(app->fpi);p[1]=fgetc(app->fpi);p[2]=fgetc(app->fpi);p[3]=fgetc(app->fpi);}
                else {p[3]=fgetc(app->fpi);p[2]=fgetc(app->fpi);p[1]=fgetc(app->fpi);p[0]=fgetc(app->fpi);}
                fprintf(fp,"%d\n",hdrv);
                fseeko64(app->fpi,241-4-headertable->selected_R+4*app->samples,1);
            }
            fclose(fp);
            fl_message("成功将目标信息输出为output.txt!");
        });
        load_hdr->callback([](Fl_Widget *,void *)
        {
            app->fcd->title("open txt");
            app->fcd->type(Fl_Native_File_Chooser::BROWSE_FILE);
            switch(app->fcd->show())
            {
                case -1:break;
                case 1:break;
                default:
                //
                app->save_hdr->activate();
            }
        });
        app->save_hdr->callback([](Fl_Widget *,void *)
        {
            int hdrv;
            unsigned char *p=(unsigned char*)&hdrv;
            fseeko64(app->fpi,3600,0);
            app->fpd=fl_fopen(app->fcd->filename(),"r");
            
            for(long long tr=0;tr<app->traces;tr++)
            {
                fseeko64(app->fpi,headertable->selected_R-1,1);
                fscanf(app->fpd,"%d",&hdrv);
                fwrite(p,4,1,app->fpd);
            }
            fclose(app->fpd);
        });
    }
    app->HdrWin->end();
    app->HdrWin->show();
}

void App::cutter_separater_cb(Fl_Widget *widget,void *)
{
    if(app->CroppingWin==NULL)
    {
        app->CroppingWin=new Fl_Window(500,200,"Cropping");
        app->CroppingWin->icon(ico);
    }
    else {app->CroppingWin->show();return;}
    Fl_Tabs *tabs=new Fl_Tabs(10,10,480,180);
    Fl_Group *CutGroup=new Fl_Group(10,35,480,155,"Cut");
    {
        char traces_c[32],samples_c[32];
        sprintf(traces_c,"%lld",app->traces);
        sprintf(samples_c,"%d",app->samples);
        app->start_trace_input=new Fl_Int_Input(110,40,120,30,"start trace:");
        app->start_trace_input->value("1");
        app->end_trace_input=new Fl_Int_Input(110,80,120,30,"end trace:");
        app->end_trace_input->value(traces_c);
        app->interval_trace_input=new Fl_Int_Input(110,120,120,30,"trace interval:");
        app->interval_trace_input->value("0");
        app->start_sample_input=new Fl_Int_Input(340,40,120,30,"start sample:");
        app->start_sample_input->value("1");
        app->end_sample_input=new Fl_Int_Input(340,80,120,30,"end sample:");
        app->end_sample_input->value(samples_c);
        app->interval_sample_input=new Fl_Int_Input(340,120,120,30,"sample interval:");
        app->interval_sample_input->value("0");
        Fl_Button *cut_but=new Fl_Button(400,160,80,25,"Cut!");
        cut_but->callback([](Fl_Widget *,void *)
        {
            long long start_trace,end_trace;
            int dtrace,dsample,start_sample,end_sample,Nsample;
            start_trace=atoi(app->start_trace_input->value());
            end_trace=atoi(app->end_trace_input->value());
            dtrace=atoi(app->interval_trace_input->value());

            start_sample=atoi(app->start_sample_input->value());
            end_sample=atoi(app->end_sample_input->value());
            dsample=atoi(app->interval_sample_input->value());
            if(start_trace<=0||end_trace>app->traces||start_trace>end_trace||start_sample<=0||end_sample>app->samples||start_sample>end_sample)
            {
                fl_message("输入参数有误");
                return;
            }
            start_trace--;start_sample--;
            char outfilename[128];
            sprintf(outfilename,"%s_sc.sgy",app->filenames.base);
            app->fpo=fl_fopen(outfilename,"wb");
            Nsample=(end_sample-start_sample)/(dsample+1.0);
            if(app->hdr[3224]==0)
            {
                app->hdr[3220]=Nsample/256;
                app->hdr[3221]=Nsample%256;
            }
            else
            {
                app->hdr[3220]=Nsample%256;
                app->hdr[3221]=Nsample/256;
            }
            fwrite(app->hdr,1,3600,app->fpo);
            float f;
            unsigned char *tbuf=(unsigned char*)malloc(240);
            fseeko64(app->fpi,3600,0);
            for(long long tr=start_trace;tr<end_trace;tr+=dtrace+1)
            {
                fseeko64(app->fpi,dtrace*(240+4*app->samples),1);
                fread(tbuf,1,240,app->fpi);
                // tbuf[114]=Nsample/256;
                // tbuf[115]=Nsample%256;
                fwrite(tbuf,1,240,app->fpo);
                fseeko64(app->fpi,start_sample*4,1);
                for(int nt=start_sample;nt<end_sample;nt+=dsample+1)
                {
                    fseeko64(app->fpi,dsample*4,1);
                    fread(&f,4,1,app->fpi);
                    fwrite(&f,4,1,app->fpo);
                }
                fseeko64(app->fpi,4*(app->samples-end_sample),1);
            }
            fclose(app->fpo);
            free(tbuf);
        });
    }
    CutGroup->end();
    Fl_Group *Split_Group=new Fl_Group(10,35,480,155,"Split");
    {
        char traces_c[32];
        sprintf(traces_c,"%lld",app->traces);
        app->n_parts_input=new Fl_Int_Input(130,50,120,30,"Number of files");
        app->n_parts_input->value("1");
        app->n_traces_input=new Fl_Int_Input(130,100,120,30,"Traces per file");
        app->n_traces_input->value(traces_c);
        Fl_Button *split_but=new Fl_Button(400,160,80,25,"Split!");
        app->n_parts_input->when(FL_WHEN_CHANGED);
        app->n_traces_input->when(FL_WHEN_CHANGED);
        app->n_parts_input->callback([](Fl_Widget *,void *)
        {
            if(0==strcmp(app->n_parts_input->value(),"\0"))return;
            long long V;
            char buf[32];
            V=atoi(app->n_parts_input->value());
            if(V<1||V>app->traces)
            {
                app->n_parts_input->value("1");
                app->n_traces_input->value("500");
                return;
            }
            sprintf(buf,"%lld",app->traces/V);
            app->n_traces_input->value(buf);
        });
        app->n_traces_input->callback([](Fl_Widget *,void *)
        {
            if(0==strcmp(app->n_traces_input->value(),"\0"))return;
            long long V;
            char buf[32];
            V=atoi(app->n_traces_input->value());
            if(V<1||V>app->traces)
            {
                app->n_parts_input->value("1");
                app->n_traces_input->value("500");
                return;
            }
            sprintf(buf,"%lld",app->traces/V);
            app->n_parts_input->value(buf);
        });
        split_but->callback([](Fl_Widget *,void *)
        {
            if(0==strcmp(app->n_traces_input->value(),"\0")||0==strcmp(app->n_parts_input->value(),"\0"))
            {
                fl_message("请输入想输入的参数喵");
                return;
            }
            int trs,trs_end,N;
            N=atoi(app->n_parts_input->value());
            trs=atoi(app->n_traces_input->value());
            trs_end=app->traces%N;
            char outfilename[128];
            // char outfolder[]="outfolder";
            fseeko64(app->fpi,3600,0);
            unsigned char *tracebuf=(unsigned char*)malloc(240+4*app->samples);
            for(int i=0;i<N+1;i++)
            {
                if(trs_end==0&&i==N)break;
                sprintf(outfilename,"%s_p%d.sgy",app->filenames.base,i+1);
                app->fpo=fl_fopen(outfilename,"wb");
                fwrite(app->hdr,1,3600,app->fpo);
                for(int j=0;j<(i==N?trs_end:trs);j++)
                {
                    fread(tracebuf,1,240+4*app->samples,app->fpi);
                    fwrite(tracebuf,1,240+4*app->samples,app->fpo);
                }
                fclose(app->fpo);
            }
            free(tracebuf);
        });
    }
    Split_Group->end();
    tabs->end();
    app->CroppingWin->show();
}

void App::format_cb(Fl_Widget *widget,void *)
{
    Fl_Menu_Bar *bar=(Fl_Menu_Bar*)widget;
    const Fl_Menu_Item *item=bar->mvalue();
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

void App::endian_cb(Fl_Widget *w,void *)
{
    Fl_Menu_Bar *bar=(Fl_Menu_Bar*)w;
    const Fl_Menu_Item *item=bar->mvalue();
    if(0==strcmp(item->label(),"big-endian"))
    {
        rt->is_le=false;
        rt->redraw();
    }
    else //if(0==strcmp(item->label(),"little-endian")
    {
        rt->is_le=true;
        rt->redraw();
    }
}

void App::enhance_cb(Fl_Widget *widget,void *)
{
    app->enhance_cnt++;
    for(int i=0;i<app->trace_s*app->samples;i++)
        app->seis[i]=app->seis[i]<0?-log(1-app->seis[i]):log(1+app->seis[i]);
    fwin->set_window_para(app->seis,app->trace_s,app->samples);
    fwin->redraw();
}

void App::attenua_cb(Fl_Widget *widget,void *)
{
    if(app->enhance_cnt==0)return;
    app->enhance_cnt--;
    for(int i=0;i<app->trace_s*app->samples;i++)
        app->seis[i]=app->seis[i]<0?1-exp(-app->seis[i]):exp(app->seis[i])-1;
    fwin->set_window_para(app->seis,app->trace_s,app->samples);
    fwin->redraw();
}

void App::load_clr_cb(Fl_Widget *,void *)
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
        app->ColorbarWin->redraw();
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
    int i,j;
    app->fc->title("Save As");
    
    app->fc->type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    switch(app->fc->show())
    {
        case -1: break;
        case 1: break;
        default:
        app->fpo=fl_fopen(app->fc->filename(),"wb");
        
        char asc_out[3200]={32};
        int tb_len=app->hdr_text->buffer()->length();
        unsigned char ebc_out[40][80]={0};
        for(i=0;i<(tb_len>3200?3200:tb_len);i++)
            asc_out[i]=app->tbuff->text()[i];

        for(i=0;i<40;i++)
        {
            for(j=0;j<80;j++)
                ebc_out[i][j]=a2e[asc_out[i*80+j]];
            fwrite(ebc_out[i],1,80,app->fpo);
        }
        fwrite(app->hdr+3200,1,400,app->fpo);
        fseeko64(app->fpi,3600,0);
        unsigned char *tbuf=(unsigned char*)malloc(240+4*app->samples);
        for(long long tr=0;tr<app->traces;tr++)
        {
            fread(tbuf,1,240+4*app->samples,app->fpi);
            fwrite(tbuf,1,240+4*app->samples,app->fpo);
        }
        free(tbuf);
        fclose(app->fpo);
    }
}

App::App(int X,int Y,int Width,int Height,const char *title):Fl_Window(X,Y,Width,Height,title)
{
    Fl::scheme("gtk+");

    icon(ico);
    begin();
    fc=new Fl_Native_File_Chooser();
    fcc=new Fl_Native_File_Chooser();
    fcd=new Fl_Native_File_Chooser();
    fc->filter("SEGY\t*.{sgy,segy,SEGY,segy}");
    fcc->filter("Text\t*.(txt,csv,clr)");
    menus=new Fl_Menu_Bar(0,0,Width,height_of_menu);
    // menus->add("File/Save as",0,save_cb,0,FL_MENU_INACTIVE);
    menus->add("File/Property",0,property_cb,0,FL_MENU_INACTIVE);
    menus->add("Tools/Textual Header Editor",0,[](Fl_Widget *,void *){app->TextWin->show();},0,FL_MENU_INACTIVE);
    menus->add("Tools/Headers",0,hdr_cb,0,FL_MENU_INACTIVE);
    menus->add("Tools/Cutter-Separater",0,cutter_separater_cb,0,FL_MENU_INACTIVE);
    menus->add("Tools/色标",0,[](Fl_Widget *,void *){app->ColorbarWin->show();},0,FL_MENU_DIVIDER);
    menus->add("Tools/图像增强","^q",enhance_cb,0,FL_MENU_INACTIVE);
    menus->add("Tools/图像减弱","^w",attenua_cb,0,FL_MENU_INACTIVE|FL_MENU_DIVIDER);
    menus->add("Tools/About",0,[](Fl_Widget *,void *){fl_open_uri("https://github.com/draw123draw/Gfimage/");});
    menus->add("data?/ibm-le",0,format_cb,0,FL_MENU_INACTIVE|FL_MENU_RADIO);
    menus->add("data?/ibm-be",0,format_cb,0,FL_MENU_INACTIVE|FL_MENU_RADIO);
    menus->add("data?/ieee-le",0,format_cb,0,FL_MENU_INACTIVE|FL_MENU_RADIO);
    menus->add("data?/ieee-be",0,format_cb,0,FL_MENU_INACTIVE|FL_MENU_RADIO|FL_MENU_DIVIDER);
    menus->add("data?/zero-centered",0,[](Fl_Widget *w,void *)
    {
        Fl_Menu_Bar *bar = (Fl_Menu_Bar*)w;
        const Fl_Menu_Item *item=bar->mvalue();
        fwin->centered_norm=item->value()?true:false;
        fwin->set_window_para(app->seis,app->trace_s,app->samples);
        fwin->redraw();
    },this,FL_MENU_INACTIVE|FL_MENU_TOGGLE); 
    
    fwin=new Imagesc(0,height_of_menu,w(),h()-height_of_menu);
    open_button=new Fl_Button((Width-open_x)/2,(Height-open_y)/2,open_x,open_y);
    fwin->hide();
    open_button->label("打开文件\n或将文件拖入窗口");
    open_button->callback(start_cb);
    resizable(*fwin);
    end();

    ColorbarWin=new Fl_Window(660,300,"Colorbar");
    ColorbarWin->icon(ico);
    ColorbarWin->begin();
    {
        const int cheight2=170,cheight3=220;
        Fl_Box *explaination=new Fl_Box(200,cheight2,220,120);
        explaination->box(FL_UP_BOX);
        explaination->align(FL_ALIGN_TOP|FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
        explaination->label("双击色标点以更改颜色,右键以删除\n\n输入数据格式:\n第一行为色标点个数\n第二行开始为0-1之间的4列实数\n第一列为色标值,后三列为r,g,b值\n以逗号隔开");
        
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
        chart=new Fl_Chart(30,90,600,70);
        chart->type(FL_BAR_CHART);
        chart->hide();
    }
    ColorbarWin->redraw();
    ColorbarWin->end();
    ColorbarWin->hide();
}

App::~App()
{
    fclose(fpi);
    free(seis);
    free(histo);
}

int App::handle(int event)
{
    int px,py;
    int ret=Fl_Window::handle(event);
    px=trace_s*Fl::event_x()/w();
    py=samples*(Fl::event_y()-30)/h();
    switch(event)
    {
        case FL_MOVE:
        if(!first_read&&Fl::belowmouse()==fwin)
        {
            sprintf(mxy,"X:%d\tY:%d",px,py);
            where_mouse_box->copy_label(mxy);
            sprintf(amp_c,"%g",seis[px*samples+py]);
            tooltip(amp_c);
        }
        break;
        case FL_DND_ENTER:
        case FL_DND_DRAG:
        case FL_DND_RELEASE:ret=1;break;
        case FL_PASTE:
        read_data(Fl::event_text());
        break;
    }
    return ret;
};

void App::open_figure()
{
    fwin->set_window_para(seis,trace_s,samples);
    fwin->redraw();
    fwin->show();
    for(int cnt=0;cnt<enhance_cnt;cnt++)
        for(int i=0;i<trace_s*samples;i++)
            seis[i]=seis[i]<0?-log(1-seis[i]):log(1+seis[i]);
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
        // item=(Fl_Menu_Item*)menus->find_item("File/Save as");item->flags=0;
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

        first_read=false;
    }
    chart->show();
    histo=(int*)calloc(bin_num,4);//bin_num==直方图区间个数
    if(fwin->max_a-fwin->min_a>FLT_MAX)return;
    for(int i=0;i<trace_s*samples;i++)
    {
        int bin=fwin->centered_norm?(int)(bin_num*0.5*(seis[i]/(fwin->max_abs_a+FLT_MIN)+1)):(int)(bin_num*(seis[i]-fwin->min_a)/(fwin->max_a-fwin->min_a+FLT_MIN));
        if(bin==bin_num)bin--;
        histo[bin]++;
    }
    for(int i=0;i<bin_num;i++)chart->add(histo[i]);
}

void App::read_data(const char *fname)
{
    int i,j;
    if(!first_read)fclose(fpi);
    fpi=fopen(fname,"rb");
    if(fpi==NULL)
    {
        fl_alert("文件不存在！输入的路径为:%s\n可尝试在主窗口中打开",fname);
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
        return;
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
    // fprintf(stderr,"samples=%d,(%d,%d)\n",samples,hdr[3220],hdr[3221]);
    traces=(l-3600)/(long long)(240+4*samples);
    trace_s=(traces<w())?traces:w();
    fseeko64(fpi,3600,0);
    seis=(float*)malloc(sizeof(float)*trace_s*samples);
    for(i=0;i<trace_s;i++)
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
    trace_num=0;
    open_figure();
    strcpy(filename,fname);
    label(fname);
    findnames(app->filename,&(app->filenames));
    if(traces>w()) //文件太大则制作slider
    {
        begin();
        trace_slider=new Fl_Value_Slider(w()-350,0,150,height_of_menu);
        trace_counter=new FreeCounter(w()-200,0,200,height_of_menu);
        trace_counter->value(1);
        end();
        trace_slider->align(FL_ALIGN_LEFT);
        trace_slider->bounds(1,traces-trace_s+1);
        trace_slider->type(1);
        trace_slider->step(1);
        trace_slider->value(1);
        trace_slider->callback(ts_cb,this);
    }
    else
    {
        Fl::delete_widget(trace_slider);
        Fl::delete_widget(trace_counter);
    }
    redraw();
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

void HeaderTable::draw_cell(TableContext context,int ROW,int COL,int X,int Y,int W,int H)
{
    static char s[32];
    switch(context)
    {
    case CONTEXT_STARTPAGE:
        fl_font(FL_HELVETICA,12);
        return;
    case CONTEXT_ROW_HEADER:
        if(ROW==0)
        {
            DrawHeader("Trace:",X,Y,W,H);
            return;
        }
        sprintf(s,"b%d",ROW);
        DrawHeader(s,X,Y,W,H);
        return;
    case CONTEXT_CELL:
        if(ROW==0)
        {
            sprintf(s,"%lld",trace_num+COL+1);
            DrawHeader(s,X,Y,W,H);
            return;
        }
        if(selected_R==0||selected_R>241-bytes_num)return;
        sprintf(s,"%d",hdrbytes_s[COL*240+ROW-1]);
        fl_push_clip(X,Y,W,H);
        if(bytes_num==4)
            fl_draw_box(FL_THIN_UP_BOX,X,Y,W,H,(ROW==selected_R||ROW==selected_R+1||ROW==selected_R+2||ROW==selected_R+3)?FL_YELLOW:FL_WHITE);
        else //if bytes_num==2
            fl_draw_box(FL_THIN_UP_BOX,X,Y,W,H,(ROW==selected_R||ROW==selected_R+1)?FL_YELLOW:FL_WHITE);
        fl_color(FL_GRAY0);fl_draw(s,X,Y,W,H,FL_ALIGN_CENTER);
        fl_pop_clip();
        return;
    default:
        return;
    }
}

void HeaderTable::DrawHeader(const char *s, int X, int Y, int W, int H)
{
    fl_push_clip(X,Y,W,H);
    fl_draw_box(FL_THIN_UP_BOX,X,Y,W,H,row_header_color());
    fl_color(FL_BLACK);
    fl_draw(s,X,Y,W,H,FL_ALIGN_CENTER);
    fl_pop_clip();
}

HeaderTable::HeaderTable(int X,int Y,int W,int H,const char *title):Fl_Table_Row(X,Y,W,H,title)
{
    hdrbytes_s=(unsigned char*)malloc(trace_s_h*240);
    rows(241);
    row_height_all(20);
    row_header(1);
    cols(trace_s_h);
    col_width_all(80);
    col_header(0);
    end();
    get_hdr();
}

void HeaderTable::get_hdr()
{
    fseeko64(app->fpi,3600+trace_num*(240+4*app->samples),0);
    for(int i=0;i<trace_s_h;i++)
    {
        fread(hdrbytes_s+i*240,1,240,app->fpi);
        fseeko64(app->fpi,app->samples*4,1);
    }
}

HeaderTable::~HeaderTable()
{
    free(hdrbytes_s);
}

int HeaderTable::handle(int event)
{
    int ret=Fl_Table_Row::handle(event);
    if(event==FL_PUSH)
    {
        for(selected_R=0;selected_R<rows();selected_R++)
            if(row_selected(selected_R))
                break;
        redraw();
        rt->redraw();
    }
    return ret;
}

VHeaderTable::VHeaderTable(int X,int Y,int W,int H,const char *title):Fl_Table(X,Y,W,H,title)
{
    fseeko64(app->fpi,3200,0);
    fread(vhdrbytes,1,400,app->fpi);
    rows(400);
    row_header(1);
    row_height_all(20);
    row_resize(0);
    cols(1);
    col_header(0);
    col_width_all(80);
    col_resize(0);
    end();
}

void VHeaderTable::draw_cell(TableContext context,int ROW,int COL,int X,int Y,int W,int H)
{
    static char s[32];
    switch(context)
    {
    case CONTEXT_STARTPAGE:
        fl_font(FL_HELVETICA, 12);
        return;
    case CONTEXT_ROW_HEADER:
        sprintf(s,"%d:",ROW+3201);
        fl_push_clip(X,Y,W,H);
        fl_draw_box(FL_THIN_UP_BOX,X,Y,W,H,row_header_color());
        fl_color(FL_BLACK);
        fl_draw(s,X,Y,W,H,FL_ALIGN_CENTER);
        fl_pop_clip();
        return;
    case CONTEXT_CELL:
        sprintf(s,"%d",vhdrbytes[ROW]);
        fl_push_clip(X,Y,W,H);
        fl_draw_box(FL_THIN_UP_BOX,X,Y,W,H,is_selected(ROW,COL)?FL_YELLOW:FL_WHITE);
        fl_color(FL_GRAY0);fl_draw(s,X,Y,W,H,FL_ALIGN_CENTER);
        fl_pop_clip();
        return;
    default:
        return;
    }
}

ResultTable::ResultTable(int X,int Y,int W,int H,const char *title):Fl_Table(X,Y,W,H,title)
{
    rows(1);
    row_header(1);
    row_height_all(20);
    row_resize(0);
    cols(headertable->trace_s_h);
    col_header(0);
    col_width_all(80);
    end();
}

void ResultTable::draw_cell(TableContext context,int ROW,int COL,int X,int Y,int W,int H)
{
    static char s[32];
    switch(context)
    {
        case CONTEXT_STARTPAGE:
            fl_font(FL_HELVETICA,12);
            return;
        case CONTEXT_ROW_HEADER:
            fl_push_clip(X,Y,W,H);
            fl_draw_box(FL_THIN_UP_BOX,X,Y,W,H,row_header_color());
            fl_color(FL_BLACK);
            fl_draw("Value:",X,Y,W,H,FL_ALIGN_CENTER);
            fl_pop_clip();
            return;
        case CONTEXT_CELL:
            fl_push_clip(X,Y,W,H);
            sprintf(s,"%d",b2i(COL));
            fl_draw_box(FL_THIN_UP_BOX,X,Y,W,H,FL_WHITE);
            fl_color(FL_GRAY0);
            fl_draw(s,X,Y,W,H,FL_ALIGN_CENTER);
            fl_pop_clip();
        default:
            return;
    }
}

int ResultTable::b2i(int C)
{
    int tnum;
    unsigned char *tbytes,tmp_b;
    tbytes=(unsigned char*)&tnum;
    tbytes[0]=headertable->hdrbytes_s[C*240+headertable->selected_R-1];
    tbytes[1]=headertable->hdrbytes_s[C*240+headertable->selected_R];
    tbytes[2]=headertable->hdrbytes_s[C*240+headertable->selected_R+1];
    tbytes[3]=headertable->hdrbytes_s[C*240+headertable->selected_R+2];
    if(!is_le)
    {
        tmp_b=tbytes[0];tbytes[0]=tbytes[3];tbytes[3]=tmp_b;
        tmp_b=tbytes[1];tbytes[1]=tbytes[2];tbytes[2]=tmp_b;
    }
    return tnum;
}

int main(int argc,char **argv)
{
    app=new App(100,100,600,900,"Open a segy file");
    app->show(1,argv);
    if(argc>1)
        app->read_data(argv[1]);
    return Fl::run();
}

void findnames(char* filename,manynames* outfile)
{   
    strcpy(outfile->path,"");
    strcpy(outfile->path_slash,"");
    strcpy(outfile->ext,"");
    int i;
    i=strlen(filename)-1;
    while(i>=0&&filename[i]!='\\'&&filename[i]!='/')i--;
    if(i>=0)
    {
        strcpy(outfile->basename,filename+i+1);
        strncpy(outfile->path,filename,i);
        strncpy(outfile->path_slash,filename,i+1);
        outfile->path[i]='\0';
        if(outfile->path[i-1]=='\\'||outfile->path[i-1]=='/')
        outfile->path[i-1]='\0';
        outfile->path_slash[i+1]='\0';
    }
    else
        strcpy(outfile->basename,filename);
    
    i=strlen(outfile->basename)-1;
    while(i>=0&&outfile->basename[i]!='.')i--;
    if(i>=0)
    {
        strncpy(outfile->base,outfile->basename,i);
        outfile->base[i]='\0';
        strcpy(outfile->ext,&outfile->basename[i+1]);
    }
    else
        strcpy(outfile->base,outfile->basename);
}
