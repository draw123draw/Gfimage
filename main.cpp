#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib,"opengl32.lib")
#pragma execution_character_set("utf-8")
#include"headers.h"
ColorPoint::ColorPoint(int x, int y, int w, int h, double r_, double g_, double b_, ColorBar* colorbar_) :Fl_Box(x, y, w, h)
{
    uchar r, g, b;
    r = r_ * 255;g = g_ * 255;b = b_ * 255;
    color(fl_rgb_color(r, g, b));
    box(FL_BORDER_BOX);
    colorbar = colorbar_;
}

int ColorPoint::handle(int event)
{
    int ret = Fl_Box::handle(event);
    int buttons = Fl::event_buttons() >> 24;
    bool is_not_side = (idx != 0 && idx != colorbar->c_num - 1);
    switch (event)
    {
    case FL_PUSH:
        if (buttons == 1 && Fl::event_clicks())
        {
            uchar r, g, b;
            Fl::get_color(color(), r, g, b);
            fl_color_chooser("choose your color", r, g, b);
            color(fl_rgb_color(r, g, b));
        }
        else if (buttons == 4 && is_not_side)colorbar->remove_color_point(idx);
        ret = 1;break;
    case FL_DRAG:
        if (is_not_side)
        {
            if (x() - colors[idx - 1]->x() < colorbar->cbox_s * 2)position(Fl::event_x() - colorbar->cbox_s, colorbar->cbox_height - 12);
            else if (colors[idx + 1]->x() - x() < colorbar->cbox_s * 2)position(Fl::event_x() - colorbar->cbox_s, colorbar->cbox_height + 12);
            else position(Fl::event_x() - colorbar->cbox_s, colorbar->cbox_height);
            colorbar->fs[idx] = (Fl::event_x() - colorbar->x()) / (float)colorbar->w();
        }
        ret = 1;break;
    case FL_RELEASE:
        if (is_not_side)
        {
            if (x() <= colors[idx - 1]->x())position(colors[idx - 1]->x() + 1, colorbar->cbox_height - 12);
            else if (x() >= colors[idx + 1]->x())position(colors[idx + 1]->x() - 1, colorbar->cbox_height + 12);
        }
        ret = 1;
    }
    redraw();
    colorbar->parent()->redraw();
    colorbar->redraw();
    if (ims)ims->redraw();
    // if(fkwin)fkwin->redraw();
    return ret;
}

ColorBar::ColorBar(int x, int y, int w, int h) :Fl_Gl_Window(x, y, w, h)
{
    box(FL_FLAT_BOX);
    set_colormap("rwb");
    sort_colors();
    end();
}

ColorBar::~ColorBar()
{
    free(fs);
    for (int i = 0;i < c_num;i++)delete colors[i];
    free(colors);
}

void ColorBar::set_colormap(const char* color_idx)
{
    if (0 == strcmp(color_idx, "gray"))
    {
        float gray_clrpts[] = { 0,0,0,0,1,1,1,1 };
        set_color_points(2, gray_clrpts);
    }
    else if (0 == strcmp(color_idx, "rainbow"))
        set_color_points(clr1_num, default_clr1);
    else if (0 == strcmp(color_idx, "kwyr"))
        set_color_points(clr2_num, default_clr2);
    else if (0 == strcmp(color_idx, "rwb"))
        set_color_points(clr3_num, default_clr3);
}

void ColorBar::set_color_points(const int clr_num, const float* defalut_clr)
{
    int i;
    for (i = 0;i < c_num;i++)
    {
        parent()->remove(colors[i]);
        delete colors[i];
    }
    c_num = clr_num;
    fs = (float*)realloc(fs, c_num * sizeof(float));
    colors = (ColorPoint**)realloc(colors, c_num * sizeof(ColorPoint*));
    parent()->begin();
    for (i = 0;i < c_num;i++)
    {
        fs[i] = defalut_clr[i * 4];
        colors[i] = new ColorPoint(x() + w() * fs[i] - cbox_s, cbox_height, cbox_s * 2, cbox_s * 2, defalut_clr[i * 4 + 1], defalut_clr[i * 4 + 2], defalut_clr[i * 4 + 3], this);
    }
    parent()->end();
    sort_colors();
    redraw();
    parent()->redraw();
    ims->redraw();
    // if(fkwin)fkwin->redraw();
}

void ColorBar::get_rgb(float f01, double& r, double& g, double& b)
{
    float t;
    int st = 0;
    uchar r1, g1, b1, r2, g2, b2;
    while (st < c_num - 1)
    {
        if (fs[st] <= f01 && f01 <= fs[st + 1])break;
        st++;
    }
    Fl::get_color(colors[st]->color(), r1, g1, b1);
    Fl::get_color(colors[st + 1]->color(), r2, g2, b2);
    t = (f01 - fs[st]) / (fs[st + 1] - fs[st]);
    r = (r1 * (1 - t) + r2 * t) / 255.0;
    g = (g1 * (1 - t) + g2 * t) / 255.0;
    b = (b1 * (1 - t) + b2 * t) / 255.0;
}

void ColorBar::remove_color_point(int idx)
{
    int i;
    Fl::delete_widget(colors[idx]);
    c_num--;
    for (i = idx;i < c_num;i++)
    {
        fs[i] = fs[i + 1];
        colors[i] = colors[i + 1];
    }
    fs = (float*)realloc(fs, c_num * sizeof(float));
    colors = (ColorPoint**)realloc(colors, c_num * sizeof(ColorPoint*));
    sort_colors();
    redraw();
    if (ims)ims->redraw();
    // if(fkwin)fkwin->redraw();
}

void ColorBar::sort_colors()
{
    float f_t;
    ColorPoint* b_t;
    int i, j;
    for (i = 0;i < c_num - 1;i++)
    {
        for (j = 0;j < c_num - i - 1;j++)
        {
            if (fs[j] > fs[j + 1])
            {
                f_t = fs[j];
                fs[j] = fs[j + 1];
                fs[j + 1] = f_t;
                b_t = colors[j];
                colors[j] = colors[j + 1];
                colors[j + 1] = b_t;
            }
        }
    }
    for (i = 0;i < c_num;i++)colors[i]->idx = i;
}

void ColorBar::draw()
{
    int i;
    double r, g, b;
    glLoadIdentity();//重置
    glOrtho(0, w(), 0, h(), -1, 1);
    glClearColor(1, 1, 1, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_QUAD_STRIP);
    for (i = 0;i < w();i++)
    {
        get_rgb(i / (float)w(), r, g, b);
        glColor3f(r, g, b);
        glVertex2i(i, 0);
        glVertex2i(i, h());
    }
    glEnd();
}

int ColorBar::handle(int event)
{
    double r, g, b;
    float fidx;
    int ret = Fl_Gl_Window::handle(event);
    switch (event)
    {
    case FL_PUSH:
        colors = (ColorPoint**)realloc(colors, (c_num + 1) * sizeof(ColorPoint*));
        fs = (float*)realloc(fs, (c_num + 1) * sizeof(float));
        fidx = (Fl::event_x()) / (float)w();
        get_rgb(fidx, r, g, b);
        parent()->begin();
        colors[c_num] = new ColorPoint(Fl::event_x() + x() - cbox_s, cbox_height, cbox_s * 2, cbox_s * 2, r, g, b, this);
        parent()->end();
        fs[c_num] = fidx;
        c_num++;
        sort_colors();
        parent()->redraw();
        return 1;
    }
    return ret;
}

Imagesc::Imagesc(int x, int y, int Width, int Height) :Fl_Gl_Window(x, y, Width, Height)
{
    initial_ims();
}

void Imagesc::initial_ims()
{
    max_a = FLT_MIN;
    min_a = FLT_MAX;
    max_abs_a = 0;
    c_norm = true, isvalid = false;
}

void Imagesc::draw()
{
    int i, j;
    glViewport(0, 0, app->w(), app->h() - app->height_of_menu);//以后要把这句话加在窗口resize的回调函数里面
    if (!isvalid)
    {
        glClearColor(1, 1, 1, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        isvalid = true;
    }
    glLoadIdentity();//重置
    glOrtho(0, app->trace_s, app->samples, 0, -1, 1);
    glBegin(GL_QUADS);
    double r, g, b;
    if (!c_norm)
    {
        float rr;
        rr = max_a - min_a;
        if (rr > FLT_MAX)
        {
            colorbar->get_rgb(0, r, g, b);
            glColor3f(r, g, b);
            glVertex2i(0, 0);
            glVertex2i(app->trace_s, 0);
            glVertex2i(app->trace_s, app->samples);
            glVertex2i(0, app->samples);
            glEnd();
            return;
        }
        for (i = 0;i < app->trace_s;i++)
        {
            for (j = 0;j < app->samples;j++)
            {
                float normdata = (app->seis[i * app->samples + j] - min_a) / (rr + FLT_MIN);
                colorbar->get_rgb(normdata, r, g, b);
                glColor3f(r, g, b);
                glVertex2i(i, j);
                glVertex2i(i + 1, j);
                glVertex2i(i + 1, j + 1);
                glVertex2i(i, j + 1);
            }
        }
    }
    else
    {
        for (i = 0;i < app->trace_s;i++)
        {
            for (j = 0;j < app->samples;j++)
            {
                float normdata = 0.5 * (app->seis[i * app->samples + j] / (max_abs_a + FLT_MIN) + 1);
                double r, g, b;
                colorbar->get_rgb(normdata, r, g, b);
                glColor3f(r, g, b);
                glVertex2i(i, j);
                glVertex2i(i + 1, j);
                glVertex2i(i + 1, j + 1);
                glVertex2i(i, j + 1);
            }
        }
    }
    glEnd();
}

void Imagesc::set_window_para()
{
    if (!c_norm)
    {
        max_a = app->seis[0];
        min_a = app->seis[0];
        for (int i = 0;i < app->trace_s * app->samples;i++)
        {
            if (max_a < app->seis[i])max_a = app->seis[i];
            if (min_a > app->seis[i])min_a = app->seis[i];
        }
    }
    else
    {
        max_abs_a = 0;
        for (int i = 0;i < app->trace_s * app->samples;i++)
            if (max_abs_a < fabs(app->seis[i]))max_abs_a = fabs(app->seis[i]);
    }
}

int Imagesc::handle(int event)
{
    int ret = Fl_Gl_Window::handle(event);
    int px, py;
    px = app->trace_s * (Fl::event_x() + 1) / w();
    py = app->samples * (Fl::event_y() + 1) / h();
    switch (event)
    {
    case FL_MOVE:
        if (!app->first_read && Fl::belowmouse() == ims)
        {
            sprintf(app->mxy, "X:%d Y:%d", px, py);
            app->where_mouse_box->value(app->mxy);
            if (px * app->samples + py < app->trace_s * app->samples)
            {
                sprintf(app->amp_c, "%g", app->seis[px * app->samples + py]);
                tooltip(app->amp_c);
            }
        }
        break;
    case FL_PUSH:
        if (Fl::event_button() == FL_RIGHT_MOUSE)
        {
            char num[64];
            sprintf(num, "%g", app->seis[px * app->samples + py]);
            expand_right_menu(Fl::event_x(), Fl::event_y(), num);
        }
        break;
    case FL_DND_ENTER:
    case FL_DND_DRAG:
    case FL_DND_RELEASE:ret = 1;break;
    case FL_PASTE:
        app->read_data(Fl::event_text());
    }
    return ret;
}

// FkWin::FkWin(int X,int Y,int Width,int Height,const char *title):Fl_Gl_Window(X,Y,Width,Height,title)
// {
//     in=(fftw_complex*)fftw_malloc(sizeof(fftw_complex)*app->trace_s*app->samples);
//     out=(fftw_complex*)fftw_malloc(sizeof(fftw_complex)*app->trace_s*app->samples);
//     fk=(float*)malloc(4*app->trace_s*app->samples);
//     for(int i=0;i<app->trace_s*app->samples;i++)
//     {
//         in[i][0]=app->seis[i];
//         in[i][1]=0;
//     }
//     p=fftw_plan_dft_2d(app->trace_s,app->samples,in,out,FFTW_FORWARD,FFTW_ESTIMATE);
//     fftw_execute(p);
//     for(int i=0;i<app->trace_s*app->samples;i++)
//     {
//         fk[i]=sqrt(out[i][0]*out[i][0]+out[i][1]*out[i][1]);
//         max_a=fk[i]>max_a?fk[i]:max_a;
//         min_a=fk[i]<min_a?fk[i]:min_a;
//     }
// }

// FkWin::~FkWin()
// {
//     fftw_free(in);
//     fftw_free(out);
//     free(fk);
// }

// void FkWin::hide()
// {
//     Fl_Gl_Window::hide();
//     delete fkwin;fkwin=NULL;
// }

// void FkWin::draw()
// {
//     glViewport(0,0,w(),h());
//     if(!isvalid)
//     {
//         glClearColor(1,1,1,0);
//         glClear(GL_COLOR_BUFFER_BIT);
//         isvalid=true;
//     }
//     glLoadIdentity();//重置
//     glOrtho(0,app->trace_s,app->samples,0,-1,1);
//     glBegin(GL_QUADS);

//     float rr;
//     double r,g,b;
//     rr=max_a-min_a;
//     if(rr>FLT_MAX)
//     {
//         colorbar->get_rgb(0,r,g,b);
//         glColor3f(r,g,b);
//         glVertex2i(0,0);
//         glVertex2i(app->trace_s,0);
//         glVertex2i(app->trace_s,app->samples);
//         glVertex2i(0,app->samples);
//         glEnd();
//         return;
//     }
//     for(int i=0;i<app->trace_s;i++)
//     {
//         for(int j=0;j<app->samples;j++)
//         {
//             float normdata=(fk[i*app->samples+j]-min_a)/(rr+FLT_MIN);
//             colorbar->get_rgb(normdata,r,g,b);
//             glColor3f(r,g,b);
//             glVertex2i(i,j);
//             glVertex2i(i+1,j);
//             glVertex2i(i+1,j+1);
//             glVertex2i(i,j+1);
//         }
//     }
//     glEnd();
// }

// int FkWin::handle(int event)
// {
//     int ret=Fl_Gl_Window::handle(event);
//     int px,py;
//     px=app->trace_s*(Fl::event_x()+1)/w();
//     py=app->samples*(Fl::event_y()+1)/h();
//     switch(event)
//     {
//         case FL_PUSH:
//         if(Fl::event_button()==FL_RIGHT_MOUSE)
//         {
//             char num[64];
//             sprintf(num,"%g",fk[px*app->samples+py]);
//             expand_right_menu(Fl::event_x(),Fl::event_y(),num);
//         }
//     }
//     return ret;
// }

HeaderWin::HeaderWin(int W, int H, const char* title) :Fl_Window(W, H, title)
{
    fcd = new Fl_Native_File_Chooser();
    fcd->filter("txt\t*.{txt}");
    default_icon(ico);
    begin();
    headertable = new HeaderTable(20, 60, 460, 440, "Trace Header");
    rt = new ResultTable(20, 520, 442, 22, 0);
    vht = new VHeaderTable(500, 60, 140, 520, "Volume Header");
    new FreeCounterHeader(70, 10, 200, 25, "Trace:");
    app->endian_menu2 = new Fl_Menu_Bar(280, 10, 200, 25);
    app->endian_menu2->add("big-endian", 0, endian_cb, 0, FL_MENU_RADIO);
    app->endian_menu2->add("little-endian", 0, endian_cb, 0, FL_MENU_RADIO);
    app->update_endian_menu(app->endian_menu2);

    check_text_but = new Fl_Button(520, 10, 100, 25, "Text Header->");
    check_text_but->callback([](Fl_Widget* w, void*)
        {
            Fl_Button* check_text_but = (Fl_Button*)w;
            if (hdrwin->w() == 660)
            {
                hdrwin->size(1200, 600);
                check_text_but->label("Text Header<-");
            }
            else
            {
                hdrwin->size(660, 600);
                check_text_but->label("Text Header->");
            }
        });
    hdr_text = new Fl_Text_Editor(670, 10, 510, 540);
    hdr_text->textsize(10);
    tbuff = new Fl_Text_Buffer();
    hdr_text->buffer(tbuff);
    unsigned char ebc[40][80];
    char asc[40][80];
    int i, j;
    fseeko64(app->fpi, 0, 0);
    for (i = 0;i < 40;i++)
    {
        fread(ebc[i], 1, 80, app->fpi);
        for (j = 0;j < 80;j++)
            asc[i][j] = e2a[ebc[i][j]];
        asc[i][79] = '\0';
        tbuff->append(asc[i]);
        tbuff->append("\n");
    }
    Fl_Button* save_but = new Fl_Button(670, 560, 100, 25, "save as");
    save_but->callback([](Fl_Widget*, void*)
        {
            int i, j;
            Fl_Native_File_Chooser native;
            native.title("Save As");
            native.filter("SEGY\t*.{sgy,segy,SEGY,segy}");
            native.preset_file("output.sgy");
            native.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
            switch (native.show())
            {
            case -1:break;
            case 1:fl_beep();break;
            default:
                if (NULL == (app->fpo = fl_fopen(native.filename(), "wb")))
                    fl_message("路径不存在，请手动创建该路径");
                char asc_out[3200] = { 32 };
                int tb_len = hdrwin->hdr_text->buffer()->length();
                unsigned char ebc_out[40][80] = { {0} };
                for (i = 0;i < (tb_len > 3200 ? 3200 : tb_len);i++)
                    asc_out[i] = hdrwin->tbuff->text()[i];
                for (i = 0;i < 40;i++)
                {
                    for (j = 0;j < 80;j++)
                        ebc_out[i][j] = a2e[(unsigned char)asc_out[i * 80 + j]];
                    fwrite(ebc_out[i], 1, 80, app->fpo);
                }
                fwrite(app->hdr + 3200, 1, 400, app->fpo);
                fseeko64(app->fpi, 3600, 0);
                unsigned char* tbuf = (unsigned char*)malloc(240 + 4 * app->samples);
                hdrwin->begin();
                Progress* progress = new Progress(800, 560, 300, 25);
                hdrwin->end();
                p100 = 0;
                for (long long tr = 0;tr < app->traces;tr++)
                {
                    check_progress(progress, tr, app->traces);
                    if (ret_flag)break;
                    fread(tbuf, 1, 240 + 4 * app->samples, app->fpi);
                    fwrite(tbuf, 1, 240 + 4 * app->samples, app->fpo);
                }
                hdrwin->remove(progress);
                hdrwin->redraw();
                free(tbuf);
                fclose(app->fpo);
            }
        });

    Fl_Button* export_hdr = new Fl_Button(20, 560, 100, 30, "export value");
    Fl_Button* load_hdr = new Fl_Button(130, 555, 100, 40, "load from\n&& save");
    app->input_load_hdr = new Fl_Input(240, 560, 200, 30);
    Fl_Button* open_but = new Fl_Button(445, 560, 40, 30, "@fileopen");
    open_but->callback([](Fl_Widget*, void*)
        {
            Fl_Native_File_Chooser native;
            native.title("open header txt");
            native.type(Fl_Native_File_Chooser::BROWSE_FILE);
            native.filter("txt\t*.{txt}");
            switch (native.show())
            {
            case -1:break;
            case 1:fl_beep();break;
            default:
                app->input_load_hdr->value(native.filename());
            }
        });
    export_hdr->callback([](Fl_Widget*, void*)
        {
            hdrwin->fcd->title("output");
            hdrwin->fcd->preset_file("output.txt");
            hdrwin->fcd->type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
            switch (hdrwin->fcd->show())
            {
            case -1:break;
            case 1:fl_beep();break;
            default:
                int hdrv;
                unsigned char* p = (unsigned char*)&hdrv;
                FILE* fp;
                if (NULL == (fp = fl_fopen(hdrwin->fcd->filename(), "w")))
                    fl_message("路径不存在，请手动创建该路径");
                fseeko64(app->fpi, 3600, 0);
                for (long long tr = 0;tr < app->traces;tr++)
                {
                    fseeko64(app->fpi, headertable->selected_R - 1, 1);
                    if (app->is_le) { p[0] = fgetc(app->fpi);p[1] = fgetc(app->fpi);p[2] = fgetc(app->fpi);p[3] = fgetc(app->fpi); }
                    else { p[3] = fgetc(app->fpi);p[2] = fgetc(app->fpi);p[1] = fgetc(app->fpi);p[0] = fgetc(app->fpi); }
                    fprintf(fp, "%d\n", hdrv);
                    fseeko64(app->fpi, 241 - 4 - headertable->selected_R + 4 * app->samples, 1);
                }
                fclose(fp);
                fl_message("输出成功!");
            }
        });
    load_hdr->callback([](Fl_Widget*, void*)
        {
            if (0 == strcmp(app->input_load_hdr->value(), ""))
            {
                fl_message("请输入按钮右侧路径喵!");
                return;
            }
            app->fc->title("save sgy");
            app->fc->preset_file("output.sgy");
            app->fc->type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
            switch (app->fc->show())
            {
            case -1:break;
            case 1:fl_beep();break;
            default:
                FILE* fp, * fps;
                if (NULL == (fp = fl_fopen(app->input_load_hdr->value(), "r")))
                    fl_message("未找到道头文件");
                if (NULL == (fps = fl_fopen(app->fc->filename(), "wb")))
                    fl_message("路径不存在，请手动创建该路径");
                int hdrv, hdr_loc;
                hdr_loc = headertable->selected_R - 1;
                unsigned char* p = (unsigned char*)&hdrv;
                unsigned char* buf = (unsigned char*)malloc(240 + 4 * app->samples);
                fseeko64(app->fpi, 3600, 0);
                fwrite(app->hdr, 3600, 1, fps);
                if (app->is_le)
                {
                    for (long long tr = 0;tr < app->traces;tr++)
                    {
                        fread(buf, 1, 240 + 4 * app->samples, app->fpi);
                        fscanf(fp, "%d", &hdrv);
                        for (int k = 0;k < 4;k++)buf[hdr_loc + k] = p[k];
                        fwrite(buf, 1, 240 + 4 * app->samples, fps);
                    }
                }
                else
                {
                    for (long long tr = 0;tr < app->traces;tr++)
                    {
                        fread(buf, 1, 240 + 4 * app->samples, app->fpi);
                        fscanf(fp, "%d", &hdrv);
                        for (int k = 0;k < 4;k++)buf[hdr_loc + k] = p[3 - k];
                        fwrite(buf, 1, 240 + 4 * app->samples, fps);
                    }
                }
                free(buf);
                fclose(fp);fclose(fps);
            }
        });
    end();
    show();
}

void HeaderWin::endian_cb(Fl_Widget* w, void*)
{
    Fl_Menu_Bar* bar = (Fl_Menu_Bar*)w;
    const Fl_Menu_Item* item = bar->mvalue();
    if (0 == strcmp(item->label(), "big-endian"))
    {
        app->is_le = false;
        if (rt)rt->redraw();
    }
    else //if(0==strcmp(item->label(),"little-endian")
    {
        app->is_le = true;
        if (rt)rt->redraw();
    }
}

Scatter::Scatter(int X, int Y, int W, int H) :Fl_Gl_Window(X, Y, W, H)
{
    xs = (int*)malloc(4 * app->traces);
    ys = (int*)malloc(4 * app->traces);
}

Scatter::~Scatter()
{
    free(xs);
    free(ys);
}

void Scatter::hide()
{
    Fl_Gl_Window::hide();
    ret_flag = true;
    delete scatter;
}

void Scatter::findmvalues(int byte1, int byte2, int& max1, int& min1, int* arr1, int& max2, int& min2, int* arr2)
{
    mapview->begin();
    Progress* progress = new Progress(160, 30, 240, 25);
    mapview->end();
    int dbyte, v, dbyte2;
    unsigned char* p, tmp;
    p = (unsigned char*)&v;
    dbyte = byte2 - byte1 - 4;
    dbyte2 = 240 + 4 * app->samples - byte2 - 3;
    p100 = 0;
    for (long long tr = 0;tr < app->traces;tr++)
    {//if写在循环里面和在循环外面，运行效率差不了太多，因此为了简化代码就写里面好了
        check_progress(progress, tr, app->traces);
        if (ret_flag)break;
        fseeko64(app->fpi, byte1 - 1, 1);
        fread(&v, 1, 4, app->fpi);
        if (!app->is_le) { tmp = p[0];p[0] = p[3];p[3] = tmp;tmp = p[1];p[1] = p[2];p[2] = tmp; }
        arr1[tr] = v;
        max1 = max1 < v ? v : max1;
        min1 = min1 > v ? v : min1;
        fseeko64(app->fpi, dbyte, 1);
        fread(&v, 1, 4, app->fpi);
        if (!app->is_le) { tmp = p[0];p[0] = p[3];p[3] = tmp;tmp = p[1];p[1] = p[2];p[2] = tmp; }
        arr2[tr] = v;
        max2 = max2 < v ? v : max2;
        min2 = min2 > v ? v : min2;
        fseeko64(app->fpi, dbyte2, 1);
    }
    mapview->remove(progress);
    mapview->redraw();
}

void Scatter::draw()
{
    if (!isvalid)
    {
        glClearColor(.5, .5, .5, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        return;
    }
    glViewport(30, 30, w() - 30, h() - 30);
    glBegin(GL_QUADS);
    glColor3f(0, 0, 0);
    glVertex2i(-1, -1);
    glVertex2i(1, -1);
    glVertex2i(1, 1);
    glVertex2i(-1, 1);
    glEnd();

    glViewport(0, 0, w(), h());
    glColor3f(1, 1, 1);
    gl_font(FL_TIMES, 12);
    gl_draw(ax_buf[0], -1.0f, .95f);
    gl_draw("x", -.95f, 0.1f);
    gl_draw(ax_buf[1], -1.0f, -.85f);
    gl_draw(ax_buf[2], -.85f, -.95f);
    gl_draw("y", 0.0f, -.95f);
    gl_draw(ax_buf[3], .9f, -.95f);
    glEnd();

    glLoadIdentity();
    glViewport(30, 30, w() - 30, h() - 30);
    glOrtho(xmin - 1, xmax + 1, ymin - 1, ymax + 1, -1, 1);
    glBegin(GL_POINTS);
    glColor3f(1, 1, 1);
    for (long long tr = 0;tr < app->traces;tr++)
        glVertex2i(xs[tr], ys[tr]);
    glEnd();
}

FreeCounterHeader::FreeCounterHeader(int X, int Y, int W, int H, const char* title) :Fl_Counter(X, Y, W, H, title)
{
    type(FL_SIMPLE_COUNTER);
    step(1);
    bounds(1, app->traces - headertable->trace_s_h + 1);
    value(1);
    align(FL_ALIGN_LEFT);
    callback([](Fl_Widget* w, void*)
        {
            Fl_Counter* fcr = (Fl_Counter*)w;
            headertable->trace_num = (long long)fcr->value() - 1;
            headertable->get_hdr();
            headertable->redraw();
            rt->redraw();
        });
};

int FreeCounterHeader::handle(int event)
{
    int ret = Fl_Counter::handle(event);
    switch (event)
    {
    case FL_PUSH:
        if (x() + w() * 3 / 16 < Fl::event_x() && Fl::event_x() < x() + w() * 13 / 16)
        {
            char ccurr[32], trace_range_s[64];
            sprintf(ccurr, "%lld", headertable->trace_num + 1);
            sprintf(trace_range_s, "current:(1-%lld)", app->traces - headertable->trace_s_h + 1);
            counter_set = new Fl_Window(300, 150, "请输入你想输入的道数喵");
            counter_set->set_modal();
            counter_set->begin();
            fi_curr = new Fl_Int_Input(120, 10, 100, 30);
            fi_curr->copy_label(trace_range_s);
            fi_curr->value(ccurr);
            set_ok = new Fl_Button(200, 50, 80, 30, "确认");
            set_ok->callback([](Fl_Widget*, void* data)
                {
                    FreeCounterHeader* fch = (FreeCounterHeader*)data;
                    long long V = atoi(fch->fi_curr->value());
                    headertable->trace_num = V - 1;
                    fch->value(V);
                    Fl::delete_widget(fch->counter_set);
                    headertable->get_hdr();
                    headertable->redraw();
                    rt->redraw();
                }, this);
            counter_set->end();
            counter_set->show();
        }
        ret = 1;
        break;
    }
    return ret;
}

void App::change_profile()
{
    fseeko64(fpi, 3600 + trace_num * (240 + 4 * samples), 0);
    for (int i = 0;i < trace_s;i++)
    {
        fseeko64(fpi, 240, 1);
        fread(seis + samples * i, 4, samples, fpi);
    }
    format_correct(seis, trace_s * samples);
    detect_nan();
    update_figure();
}

void App::change_profile_bin()
{
    fseeko64(fpi, trace_num * 4 * samples, 0);
    for (int i = 0;i < trace_s;i++)
        fread(seis + samples * i, 4, samples, fpi);
    format_correct(seis, trace_s * samples);
    detect_nan();
    update_figure();
}

void App::format_correct(float* seis_, int length)
{//fmt, 0= ieee-le, 1= ibm-le, 2= ieee-be, 3= ibm-be, 4= int-le, 5= int32-be
    unsigned char* p, * pi;
    int seis_i;
    pi = (unsigned char*)&seis_i;
    switch (fmk)
    {
    case 0:break;
    case 1:
        ieee2ibm(seis_, length);
        swap_bytes(seis_, length);
        break;
    case 2:
        swap_bytes(seis_, length);
        break;
    case 3:
        ieee2ibm(seis_, length);
        break;
    case 4:
        for (int i = 0;i < length;i++)
        {
            p = (unsigned char*)&seis_[i];
            for (int k = 0;k < 4;k++)pi[k] = p[k];
            seis_[i] = seis_i;
        }
        break;
    case 5:
        for (int i = 0;i < length;i++)
        {
            p = (unsigned char*)&seis_[i];
            for (int k = 0;k < 4;k++)pi[k] = p[3 - k];
            seis_[i] = seis_i;
        }
        break;
    }
}

void App::swap_bytes(float* seis_b, int length)
{
    unsigned char* p, tmp_c;
    for (long long tr = 0;tr < length;tr++)
    {
        p = (unsigned char*)&seis_b[tr];
        tmp_c = p[0], p[0] = p[3];p[3] = tmp_c;
        tmp_c = p[1], p[1] = p[2];p[2] = tmp_c;
    }
}

void App::update_endian_menu(Fl_Menu_Bar* endian_menu)
{
    if (app->is_le)
    {
        Fl_Menu_Item* item = (Fl_Menu_Item*)endian_menu->find_item("little-endian");
        item->set(); //默认为big-endian
    }
    else
    {
        Fl_Menu_Item* item = (Fl_Menu_Item*)endian_menu->find_item("big-endian");
        item->set(); //默认为big-endian
    }
}

void Property::global_but_cb(Fl_Widget*, void*)
{
    fseeko64(app->fpi, 3600, 0);
    float* strace = (float*)malloc(app->samples * 4);
    property->begin();
    Progress* progress = new Progress(200, 300, 240, 30);
    property->end();
    p100 = 0;
    for (long long tr = 0;tr < app->traces;tr++)
    {
        check_progress(progress, tr, app->traces);
        if (ret_flag)break;
        fseeko64(app->fpi, 240, 1);
        fread(strace, 4, app->samples, app->fpi);
        app->format_correct(strace, app->samples);
        for (int j = 0;j < app->samples;j++)
        {
            property->fmax = property->fmax < strace[j] ? strace[j] : property->fmax;
            property->fmin = property->fmin > strace[j] ? strace[j] : property->fmin;
        }
    }
    property->remove(progress);
    property->redraw();
    free(strace);
    sprintf(property->global_text, "最大值为:%g\n最小值为:%g", property->fmax, property->fmin);
    property->global_box->copy_label(property->global_text);
}

void Property::hide()
{
    Fl_Window::hide();
    ret_flag = true;
}

Property::Property(int W, int H, const char* title) :Fl_Window(W, H, title)
{
    default_icon(ico);
    float dt;
    dt = (app->hdr[3224] == 0) ? (app->hdr[3216] * 256 + app->hdr[3217]) : (app->hdr[3216] + app->hdr[3217] * 256);
    dt /= 1000;
    char size_text[32];
    if (app->l < 1024 * 1024)sprintf(size_text, "%.3lf KB", app->l / 1024.0);
    else if (app->l > 1024 * 1024 && app->l < 1024 * 1024 * 1024)sprintf(size_text, "%.3lf MB", app->l / (1024.0 * 1024));
    else if (app->l > 1024 * 1024 * 1024 && app->l < 1024ULL * 1024 * 1024 * 1024)sprintf(size_text, "%.3lf GB", app->l / (1024.0 * 1024 * 1024));
    else sprintf(size_text, "%.3lf TB", app->l / (1024.0 * 1024 * 1024 * 1024));
    char property_text[512];
    sprintf(property_text, "文件大小:%s\nsamples:%d,信息来自卷头3220,3221分别为(%d,%d)\ntraces:%lld\ndt:%g ms\n",
        size_text, app->samples, app->hdr[3220], app->hdr[3221], app->traces, dt);
    begin();
    text_box = new Fl_Box(10, 10, 480, 280);
    text_box->align(FL_ALIGN_TOP_LEFT | FL_ALIGN_INSIDE);
    text_box->box(FL_UP_BOX);
    text_box->copy_label(property_text);
    global_box = new Fl_Box(10, 340, 480, 150);
    global_box->align(FL_ALIGN_TOP_LEFT | FL_ALIGN_INSIDE);
    global_box->box(FL_UP_BOX);
    global_box->label("最大值为:\n最小值为:");
    global_but = new Fl_Button(30, 300, 150, 30, "Get Global Values");
    global_but->callback(global_but_cb);
    end();
    show();
}

Progress::Progress(int X, int Y, int W, int H) :Fl_Progress(X, Y, W, H)
{
    stop = false;
}

int Progress::handle(int event)
{
    if (event == FL_PUSH)stop = true;
    return Fl_Progress::handle(event);
}

void Cutter::hide()
{
    Fl_Window::hide();
    ret_flag = true;
}

Cutter::Cutter(int W, int H, const char* title) :Fl_Window(W, H, title)
{
    default_icon(ico);
    strcpy(outfilename, app->filenames.path_slash);
    strcat(outfilename, app->filenames.base);
    strcpy(outfolder, app->filenames.path_slash);
    strcat(outfilename, "_sc.sgy");
    tabs = new Fl_Tabs(10, 10, 480, 180);
    CutGroup = new Fl_Group(10, 35, 480, 155, "Cut");
    {
        input_cut = new Fl_Input(80, 160, 250, 25, "输出路径:");
        input_cut->value(outfilename);
        open_file_cut = new Fl_Button(330, 160, 40, 25, "@fileopen");
        open_file_cut->callback([](Fl_Widget*, void*)
            {
                Fl_Native_File_Chooser native;
                native.title("输出文件");
                native.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
                native.filter("SEGY\t*.*");
                switch (native.show())
                {
                case -1:break;
                case 1:fl_beep();break;
                default:
                    strcpy(cutter->outfilename, native.filename());
                    cutter->input_cut->value(cutter->outfilename);
                }
            });
        start_trace_input = new Fl_Int_Input(110, 40, 120, 30, "start trace:");
        start_trace_input->value("1");
        end_trace_input = new Fl_Int_Input(110, 80, 120, 30, "end trace:");
        end_trace_input->value(app->traces_c);
        interval_trace_input = new Fl_Int_Input(110, 120, 120, 30, "trace interval:");
        interval_trace_input->value("0");
        start_sample_input = new Fl_Int_Input(340, 40, 120, 30, "start sample:");
        start_sample_input->value("1");
        end_sample_input = new Fl_Int_Input(340, 80, 120, 30, "end sample:");
        end_sample_input->value(app->samples_c);
        interval_sample_input = new Fl_Int_Input(340, 120, 120, 30, "sample interval:");
        interval_sample_input->value("0");
        Fl_Button* cut_but = new Fl_Button(400, 160, 80, 25, "Cut!");
        cut_but->callback(cut_cb);
    }
    CutGroup->end();
    SplitGroup = new Fl_Group(10, 35, 480, 155, "Split");
    {
        input_split = new Fl_Input(80, 160, 250, 25, "输出路径:");
        input_split->value(outfolder);
        open_file_split = new Fl_Button(330, 160, 40, 25, "@fileopen");
        open_file_split->callback([](Fl_Widget*, void*)
            {
                Fl_Native_File_Chooser native;
                native.title("输出路径");
                native.type(Fl_Native_File_Chooser::BROWSE_SAVE_DIRECTORY);
                switch (native.show())
                {
                case -1:break;
                case 1:fl_beep();break;
                default:
                    strcpy(cutter->outfolder, native.filename());
                    cutter->input_cut->value(cutter->outfolder);
                    //创建文件夹
                }
            });
        n_parts_input = new Fl_Int_Input(130, 50, 120, 30, "Number of files");
        n_parts_input->value("1");
        n_traces_input = new Fl_Int_Input(130, 100, 120, 30, "Traces per file");
        n_traces_input->value(app->traces_c);
        Fl_Button* split_but = new Fl_Button(400, 160, 80, 25, "Split!");
        n_parts_input->when(FL_WHEN_CHANGED);
        n_traces_input->when(FL_WHEN_CHANGED);
        n_parts_input->callback([](Fl_Widget*, void*)
            {
                if (0 == strcmp(cutter->n_parts_input->value(), "\0"))return;
                long long V;
                char buf[32];
                V = atoi(cutter->n_parts_input->value());
                if (V<1 || V>app->traces)
                {
                    cutter->n_parts_input->value("1");
                    cutter->n_traces_input->value(app->traces_c);
                    return;
                }
                sprintf(buf, "%lld", app->traces / V);
                cutter->n_traces_input->value(buf);
            });
        n_traces_input->callback([](Fl_Widget*, void*)
            {
                if (0 == strcmp(cutter->n_traces_input->value(), "\0"))return;
                long long V;
                char buf[32];
                V = atoi(cutter->n_traces_input->value());
                if (V<1 || V>app->traces)
                {
                    cutter->n_parts_input->value("1");
                    cutter->n_traces_input->value(app->traces_c);
                    return;
                }
                sprintf(buf, "%lld", app->traces / V);
                cutter->n_parts_input->value(buf);
            });
        split_but->callback(split_cb);
    }
    SplitGroup->end();
    tabs->end();
    show();
}

void Converter::format_cb(Fl_Widget* w, void*)
{
    Fl_Menu_Bar* bar = (Fl_Menu_Bar*)w;
    const Fl_Menu_Item* item = bar->mvalue();
    if (0 == strcmp(item->label(), "2工作站格式"))
    {
        converter->sun2pc = false;
        sprintf(converter->outfilename, "%s%s_sun.sgy", app->filenames.path_slash, app->filenames.base);
        converter->input->value(converter->outfilename);
    }
    else //if(0==strcmp(item->label(),"little-endian")
    {
        converter->sun2pc = true;
        sprintf(converter->outfilename, "%s%s_pc.sgy", app->filenames.path_slash, app->filenames.base);
        converter->input->value(converter->outfilename);
    }
}

void Converter::bformat_cb(Fl_Widget* w, void*)
{
    Fl_Menu_Bar* bar = (Fl_Menu_Bar*)w;
    const Fl_Menu_Item* item = bar->mvalue();
    if (0 == strcmp(item->label(), "npy"))
    {
        converter->bformat = 0;
        sprintf(converter->outfilename, "%s%s.npy", app->filenames.path_slash, app->filenames.base);
        converter->binput->value(converter->outfilename);
    }
    else if (0 == strcmp(item->label(), "mat"))
    {
        converter->bformat = 1;
        sprintf(converter->outfilename, "%s%s.mat", app->filenames.path_slash, app->filenames.base);
        converter->binput->value(converter->outfilename);
    }
}

void Cutter::cut_cb(Fl_Widget*, void*)
{
    cutter->begin();
    Progress* progress = new Progress(150, 10, 240, 25);
    cutter->end();
    long long start_trace, end_trace;
    int dtrace, dsample, start_sample, end_sample, Nsample;
    start_trace = atoi(cutter->start_trace_input->value());
    end_trace = atoi(cutter->end_trace_input->value());
    dtrace = atoi(cutter->interval_trace_input->value());
    start_sample = atoi(cutter->start_sample_input->value());
    end_sample = atoi(cutter->end_sample_input->value());
    dsample = atoi(cutter->interval_sample_input->value());
    if (start_trace <= 0 || end_trace > app->traces || start_trace > end_trace || start_sample <= 0 || end_sample > app->samples || start_sample > end_sample)
    {
        fl_message("输入参数有误,道数应该在1到%lld之间,采样点数应该在1到%d之间", app->traces, app->samples);
        return;
    }
    start_trace--;start_sample--;
    if (NULL == (app->fpo = fl_fopen(cutter->input_cut->value(), "wb")))
        fl_message("路径不存在,请手动创建该路径");
    Nsample = ceil((end_sample - start_sample) / (dsample + 1.0));
    unsigned char whdr[3600];
    unsigned char tbuf[240];
    memcpy(whdr, app->hdr, 3600);
    if (whdr[3224] == 0)
    {
        whdr[3220] = Nsample / 256;
        whdr[3221] = Nsample % 256;
    }
    else
    {
        whdr[3220] = Nsample % 256;
        whdr[3221] = Nsample / 256;
    }
    fwrite(whdr, 1, 3600, app->fpo);
    float f;
    fseeko64(app->fpi, 3600 + start_trace * (240 + 4 * app->samples), 0);
    p100 = 0;
    for (long long tr = start_trace;tr < end_trace;tr += dtrace + 1)
    {
        check_progress(progress, tr - start_trace, end_trace - start_trace);
        if (ret_flag)break;
        fread(tbuf, 1, 240, app->fpi);
        fwrite(tbuf, 1, 240, app->fpo);
        fseeko64(app->fpi, start_sample * 4, 1);
        for (int nt = start_sample;nt < end_sample;nt += dsample + 1)
        {
            fread(&f, 4, 1, app->fpi);
            fwrite(&f, 4, 1, app->fpo);
            fseeko64(app->fpi, dsample * 4, 1);
        }
        fseeko64(app->fpi, 4 * (((end_sample - start_sample) % (dsample + 1)) - dsample - 1) + 4 * (app->samples - end_sample) + dtrace * (240 + 4 * app->samples), 1);
    }
    cutter->remove(progress);
    cutter->redraw();
    fclose(app->fpo);
}

void Cutter::split_cb(Fl_Widget*, void*)
{
    if (0 == strcmp(cutter->n_traces_input->value(), "\0") || 0 == strcmp(cutter->n_parts_input->value(), "\0"))
    {
        fl_message("请输入想输入的参数喵");
        return;
    }
    int trs, trs_end, N;
    N = atoi(cutter->n_parts_input->value());
    trs = atoi(cutter->n_traces_input->value());
    trs_end = app->traces % N;
    char folder_outfilename[256];
    char input[256];
    strcpy(input, cutter->input_split->value());
    if (input[strlen(input) - 1] != '\\')
        strcat(input, "\\");
    fl_mkdir(input, 0);
    fseeko64(app->fpi, 3600, 0);
    unsigned char* tracebuf = (unsigned char*)malloc(240 + 4 * app->samples);
    for (int i = 0;i < N + 1;i++)
    {
        if (trs_end == 0 && i == N)break;
        sprintf(folder_outfilename, "%s%s_p%d.sgy", input, app->filenames.base, i + 1);
        if (NULL == (app->fpo = fl_fopen(folder_outfilename, "wb")))
            fl_message("路径不存在，请手动创建该路径");
        fwrite(app->hdr, 1, 3600, app->fpo);
        for (int j = 0;j < (i == N ? trs_end : trs);j++)
        {
            fread(tracebuf, 1, 240 + 4 * app->samples, app->fpi);
            fwrite(tracebuf, 1, 240 + 4 * app->samples, app->fpo);
        }
        fclose(app->fpo);
    }
    free(tracebuf);
}

void Converter::convert_cb(Fl_Widget*, void*)
{
    int i, k;
    p100 = 0;
    converter->begin();
    Progress* progress = new Progress(50, 160, 240, 25);
    converter->end();
    unsigned char vheader[3600], theader[240], tc, E, * p;
    fseek(app->fpi, 0, 0);
    fread(vheader, 1, 3600, app->fpi);
    char sgn;
    float ft, f = 0;
    if (NULL == (app->fpo = fl_fopen(converter->input->value(), "wb")))
        fl_message("路径不存在，请手动创建该路径");
    unsigned char* buf = (unsigned char*)malloc(app->samples * 4);
    if (converter->sun2pc)
    {
        if (vheader[3224] + vheader[3225] != 1)
            fl_message("warning:The file format is:%d,%d\n", vheader[3224], vheader[3225]);
        //根据刘磊的规则搞的
        for (i = 3200;i < 3212;i += 4)
        {
            tc = vheader[i];vheader[i] = vheader[i + 3];vheader[i + 3] = tc;
            tc = vheader[i + 1];vheader[i + 1] = vheader[i + 2];vheader[i + 2] = tc;
        }
        for (i = 3212;i < 3260;i += 2)
        {
            tc = vheader[i];vheader[i] = vheader[i + 1];vheader[i + 1] = tc;
        }
        vheader[3224] = 5;
        vheader[3225] = 0;
        fwrite(vheader, 1, 3600, app->fpo);
        for (long long tr = 0;tr < app->traces;tr++)
        {
            check_progress(progress, tr, app->traces);
            if (ret_flag)break;
            fread(theader, 1, 240, app->fpi);
            //全给换了得了，2位的换下位置就ok
                        // for(i=0;i<240;i+=4)
                        // {
                        //     tc=theader[i];theader[i]=theader[i+3];theader[i+3]=tc;
                        //     tc=theader[i+1];theader[i+1]=theader[i+2];theader[i+2]=tc;
                        // }
            fwrite(theader, 1, 240, app->fpo);
            fread(buf, 1, 4 * app->samples, app->fpi);
            //这里先获取的振幅再根据振幅转ibm
            for (i = 0;i < 4 * app->samples;i += 4)
            {
                sgn = 1;
                if (buf[i] >= 128)
                {
                    sgn = -1;
                    buf[i] = buf[i] - 128;
                }
                f = sgn * ((buf[i + 1] << 16) + (buf[i + 2] << 8) + buf[i + 3]) * pow_table[buf[i]];
                p = (unsigned char*)&f;
                for (k = 0;k < 4;k++)buf[i + k] = p[k];
            }
            fwrite(buf, 1, 4 * app->samples, app->fpo);
        }
    }
    else
    {
        if (vheader[3224] + vheader[3225] != 5)
            fl_message("warning:The file format is:%d,%d\n", vheader[3224], vheader[3225]);
        unsigned char* p, * pfn;
        unsigned fn;
        //根据刘磊的规则搞的
        for (i = 3200;i < 3212;i += 4)
        {
            tc = vheader[i];vheader[i] = vheader[i + 3];vheader[i + 3] = tc;
            tc = vheader[i + 1];vheader[i + 1] = vheader[i + 2];vheader[i + 2] = tc;
        }
        for (i = 3212;i < 3260;i += 2)
        {
            tc = vheader[i];vheader[i] = vheader[i + 1];vheader[i + 1] = tc;
        }
        vheader[3224] = 0;
        vheader[3225] = 1;
        fwrite(vheader, 1, 3600, app->fpo);
        for (long long tr = 0;tr < app->traces;tr++)
        {
            check_progress(progress, tr, app->traces);
            if (ret_flag)break;
            fread(theader, 1, 240, app->fpi);
            // for(i=0;i<240;i+=4)
            // {
            //     tc=theader[i];theader[i]=theader[i+3];theader[i+3]=tc;
            //     tc=theader[i+1];theader[i+1]=theader[i+2];theader[i+2]=tc;
            // }
            fwrite(theader, 1, 240, app->fpo);
            fread(buf, 1, 4 * app->samples, app->fpi);
            //这里先获取的振幅再根据振幅转ibm
            for (i = 0;i < 4 * app->samples;i += 4)
            {
                p = (unsigned char*)&f;
                for (k = 0;k < 4;k++)p[k] = buf[i + k];
                if ((ft = f) != 0)
                {
                    pfn = (unsigned char*)&fn;
                    for (k = 0;k < 4;k++)pfn[k] = p[k];
                    sgn = (pfn[3] >> 7);
                    E = 64;
                    if (fabs(ft) >= 1)
                    {
                        while ((int)ft != 0)
                        {
                            E++;
                            ft /= 16;
                        }
                    }
                    else
                    {
                        E++;
                        while ((int)ft * 16 == 0)
                        {
                            E--;
                            ft *= 16;
                        }
                    }
                    fn = ((sgn << 31) | (E << 24)) | (unsigned)((1 - 2 * sgn) * f * pow_table[E]);
                    for (k = 0;k < 4;k++)buf[i + k] = pfn[3 - k];
                }
            }
            fwrite(buf, 1, 4 * app->samples, app->fpo);
        }
    }
    free(buf);
    converter->remove(progress);
    converter->redraw();
    fclose(app->fpo);
}

void Converter::bconvert_cb(Fl_Widget*, void*)
{
    Progress* progress = new Progress(50, 160, 240, 25);
    p100 = 0;
    if (NULL == (app->fpo = fl_fopen(converter->binput->value(), "wb")))
        fl_message("路径不存在，请手动创建该路径");
    if (converter->bformat == 0)
    {
        fwrite("\x93NUMPY\x01\x00", 1, 8, app->fpo);
        unsigned header_size = 128 - 10;
        fwrite(&header_size, 1, 2, app->fpo);
        fprintf(app->fpo, "{'descr': '<f4', 'fortran_order': False, 'shape': (%lld, %d), }", app->traces, app->samples);
        fprintf(app->fpo, "%*s\n", 128 - 1 - (int)ftell(app->fpo), "");
        float* strace = (float*)malloc(app->samples * 4);
        fseeko64(app->fpi, 3600, 0);
        for (long long tr = 0;tr < app->traces;tr++)
        {
            check_progress(progress, tr, app->traces);
            if (ret_flag)break;
            fseeko64(app->fpi, 240, 1);
            fread(strace, 4, app->samples, app->fpi);
            app->format_correct(strace, app->samples);
            fwrite(strace, 4, app->samples, app->fpo);
        }
        free(strace);
        fclose(app->fpo);
    }
    else if (converter->bformat == 1)
    {
        fl_message("开发中...");
        return;
    }
    converter->remove(progress);
    converter->redraw();
}

void Converter::hide()
{
    Fl_Window::hide();
    ret_flag = true;
}

Converter::Converter(int W, int H, const char* title) :Fl_Window(W, H, title)
{
    default_icon(ico);
    tabs = new Fl_Tabs(10, 10, 480, 180);
    Fl_Menu_Item* item;
    fgroup = new Fl_Group(10, 35, 480, 165, "pc&&sun");
    {
        notes = new Fl_Box(25, 45, W - 50, 30);
        notes->box(FL_DOWN_BOX);
        notes->label("原始格式取自文件卷头,可在菜单Tools/Headers中查看");
        Fl_Menu_Bar* format_menu = new Fl_Menu_Bar(25, 85, 220, 30);
        format_menu->add("2工作站格式", 0, format_cb, 0, FL_MENU_RADIO);
        format_menu->add("2微机格式", 0, format_cb, 0, FL_MENU_RADIO);
        input = new Fl_Input(90, 125, 320, 25, "输出路径:");
        if (app->hdr[3224] + app->hdr[3225] == 5)
        {
            item = (Fl_Menu_Item*)format_menu->find_item("2工作站格式");
            sun2pc = false;
            item->set();
            sprintf(outfilename, "%s%s_sun.sgy", app->filenames.path_slash, app->filenames.base);
            input->value(outfilename);
        }
        else
        {
            item = (Fl_Menu_Item*)format_menu->find_item("2微机格式");
            sun2pc = true;
            item->set();
            sprintf(outfilename, "%s%s_pc.sgy", app->filenames.path_slash, app->filenames.base);
            input->value(outfilename);
        }
        open_file_format = new Fl_Button(410, 125, 40, 25, "@fileopen");
        open_file_format->callback([](Fl_Widget*, void*)
            {
                Fl_Native_File_Chooser native;
                native.title("输出文件");
                native.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
                native.filter("SEGY\t*.*");
                switch (native.show())
                {
                case -1:break;
                case 1:fl_beep();break;
                default:
                    converter->input->value(native.filename());
                }
            });
        convert_ok = new Fl_Button(W - 115, H - 40, 100, 25, "Convert!");
        convert_ok->callback(convert_cb);
    }
    fgroup->end();
    bgroup = new Fl_Group(10, 35, 480, 165, "2binary");
    {
        notes = new Fl_Box(25, 45, W - 50, 30);
        notes->box(FL_DOWN_BOX);
        notes->label("npy为python常用格式,mat为matlab常用格式");
        Fl_Menu_Bar* format_menu = new Fl_Menu_Bar(25, 85, 220, 30);
        format_menu->add("npy", 0, bformat_cb, 0, FL_MENU_RADIO);
        format_menu->add("mat", 0, bformat_cb, 0, FL_MENU_RADIO);
        binput = new Fl_Input(90, 125, 320, 25, "输出路径:");
        item = (Fl_Menu_Item*)format_menu->find_item("npy");
        item->set();
        sprintf(outfilename, "%s%s.npy", app->filenames.path_slash, app->filenames.base);
        binput->value(outfilename);
        open_file_bformat = new Fl_Button(410, 125, 40, 25, "@fileopen");
        open_file_bformat->callback([](Fl_Widget*, void*)
            {
                Fl_Native_File_Chooser native;
                native.title("输出文件");
                native.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
                native.filter("npy\t*.{npy}");
                switch (native.show())
                {
                case -1:break;
                case 1:fl_beep();break;
                default:
                    converter->binput->value(native.filename());
                }
            });
        bconvert_ok = new Fl_Button(W - 115, H - 40, 100, 25, "Convert!");
        bconvert_ok->callback(bconvert_cb);
    }
    show();
}

void App::format_cb(Fl_Widget* widget, void*)
{
    Fl_Menu_Bar* bar = (Fl_Menu_Bar*)widget;
    const Fl_Menu_Item* item = bar->mvalue();
    char ipath[64];
    bar->item_pathname(ipath, sizeof(ipath));
    if (0 == strcmp(item->label(), "ibm-le"))app->fmk = 1;
    else if (0 == strcmp(item->label(), "ibm-be"))app->fmk = 3;
    else if (0 == strcmp(item->label(), "ieee-le"))app->fmk = 0;
    else if (0 == strcmp(item->label(), "ieee-be"))app->fmk = 2;
    else if (0 == strcmp(item->label(), "int32-le"))app->fmk = 4;
    else if (0 == strcmp(item->label(), "int32-be"))app->fmk = 5;
    app->change_profile();
}

void App::enhance_cb(Fl_Widget*, void*)
{
    app->enhance_cnt++;
    for (int i = 0;i < app->trace_s * app->samples;i++)
        app->seis[i] = app->seis[i] < 0 ? -logf(1 - app->seis[i]) : logf(1 + app->seis[i]);
    ims->set_window_para();
    ims->redraw();
}

void App::attenua_cb(Fl_Widget*, void*)
{
    if (app->enhance_cnt == 0)return;
    app->enhance_cnt--;
    for (int i = 0;i < app->trace_s * app->samples;i++)
        app->seis[i] = app->seis[i] < 0 ? 1 - expf(-app->seis[i]) : expf(app->seis[i]) - 1;
    ims->set_window_para();
    ims->redraw();
}

void App::load_clr_cb(Fl_Widget*, void*)
{
    app->fcc->title("Load colormap");
    app->fcc->preset_file("");
    app->fcc->type(Fl_Native_File_Chooser::BROWSE_FILE);
    switch (app->fcc->show())
    {
    case -1:break;
    case 1:fl_beep();break;
    default:
        FILE* fpc;
        int find_c_num;
        fpc = fl_fopen(app->fcc->filename(), "r");
        fscanf(fpc, "%d", &find_c_num);
        float* custom_color = (float*)malloc(16 * colorbar->c_num);
        for (int i = 0;i < colorbar->c_num * 4;i++)
            fscanf(fpc, "%f,", &custom_color[i]);
        colorbar->set_color_points(find_c_num, custom_color);
        colorbar->sort_colors();
        colorbar->redraw();
        ims->redraw();
        app->ColorbarWin->redraw();
        // if(fkwin)fkwin->redraw();
        fclose(fpc);
        free(custom_color);
    }
}

void App::save_clr_cb(Fl_Widget*, void*)
{
    app->fcc->title("Save colormap as");
    app->fcc->preset_file("output.txt");
    app->fcc->type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    switch (app->fcc->show())
    {
    case -1:break;
    case 1:fl_beep();break;
    default:
        FILE* fpc;
        if (NULL == (fpc = fl_fopen(app->fcc->filename(), "w")))
            fl_message("路径不存在，请手动创建该路径");
        fprintf(fpc, "%d\n", colorbar->c_num);
        for (int i = 0;i < colorbar->c_num;i++)
        {
            uchar r, g, b;
            Fl::get_color(colors[i]->color(), r, g, b);
            fprintf(fpc, "%g\t%g\t%g\t%g\n", colorbar->fs[i], r / 255.0, g / 255.0, b / 255.0);
        };
        fclose(fpc);
        fl_message("输出成功!");
    }
}

App::App(int X, int Y, int Width, int Height, const char* title) :Fl_Window(X, Y, Width, Height, title)
{
    Fl::scheme("gtk+");
    default_icon(ico);
    begin();

    fc = new Fl_Native_File_Chooser();
    fc->filter("SEGY\t*.{sgy,segy,SEGY,segy}");
    fcc = new Fl_Native_File_Chooser();
    fcc->filter("Text\t*.{txt,csv,clr}");

    menus = new Fl_Menu_Bar(0, 0, Width, height_of_menu);
    menus->add("Property", 0, [](Fl_Widget*, void*)
        {
            if (property == NULL)property = new Property(500, 500, "property");
            else { property->show();return; }
        }, 0, FL_MENU_INACTIVE);
    menus->add("Tools/Headers", 0, [](Fl_Widget*, void*)
        {
            if (hdrwin == NULL)hdrwin = new HeaderWin(660, 600, "Headers");
            else { hdrwin->show();return; }
        }, 0, FL_MENU_INACTIVE);
    menus->add("Tools/Cutter-Separater", 0, [](Fl_Widget*, void*)
        {
            if (cutter == NULL)cutter = new Cutter(500, 200, "Cropping");
            else { cutter->show();return; }
        }, 0, FL_MENU_INACTIVE);
    menus->add("Tools/Format Converting", 0, [](Fl_Widget*, void*)
        {
            if (converter == NULL)converter = new Converter(500, 200, "Format Converting");
            else { converter->show();return; }
        }, 0, FL_MENU_INACTIVE);
    menus->add("Tools/Map view", 0, [](Fl_Widget*, void*)
        {
            if (mapview == NULL)mapview = new MapViewWin(500, 500, "map view");
            else { mapview->show();return; }
        }, 0, FL_MENU_INACTIVE);
    // menus->add("Tools/F-k View",0,[](Fl_Widget *,void *)
    // {
    //     fkwin=new FkWin(10,10,600,600,"F-k View");
    //     fkwin->show();
    // },0,FL_MENU_INACTIVE|FL_MENU_DIVIDER);
    menus->add("Tools/图像增强", "^q", enhance_cb, 0, FL_MENU_INACTIVE);
    menus->add("Tools/图像减弱", "^w", attenua_cb, 0, FL_MENU_INACTIVE | FL_MENU_DIVIDER);
    // menus->add("Tools/Zoomer",0,0,0,FL_MENU_INACTIVE);
    // menus->add("Tools/还原",0,0,0,FL_MENU_INACTIVE|FL_MENU_DIVIDER);
    menus->add("Tools/关于", 0, [](Fl_Widget*, void*) {fl_open_uri("https://github.com/draw123draw/Gfimage/");}, 0, 0);
    menus->add("data?/ibm-le", 0, format_cb, 0, FL_MENU_INACTIVE | FL_MENU_RADIO);
    menus->add("data?/ibm-be", 0, format_cb, 0, FL_MENU_INACTIVE | FL_MENU_RADIO);
    menus->add("data?/ieee-le", 0, format_cb, 0, FL_MENU_INACTIVE | FL_MENU_RADIO);
    menus->add("data?/ieee-be", 0, format_cb, 0, FL_MENU_INACTIVE | FL_MENU_RADIO);
    menus->add("data?/int32-le", 0, format_cb, 0, FL_MENU_INACTIVE | FL_MENU_RADIO);
    menus->add("data?/int32-be", 0, format_cb, 0, FL_MENU_INACTIVE | FL_MENU_RADIO | FL_MENU_DIVIDER);
    menus->add("data?/zero-centered", 0, [](Fl_Widget* w, void*)
        {
            Fl_Menu_Bar* bar = (Fl_Menu_Bar*)w;
            const Fl_Menu_Item* item = bar->mvalue();
            ims->c_norm = item->value() ? true : false;
            ims->set_window_para();
            ims->redraw();
        }, this, FL_MENU_INACTIVE | FL_MENU_TOGGLE);
    tc_step = new Fl_Int_Input(w() - 35, 0, 35, height_of_menu, "±");
    tc_step->when(FL_WHEN_CHANGED);
    tc_step->value("1");
    tc_step->callback([](Fl_Widget* w, void*)
        {
            Fl_Int_Input* tcs = (Fl_Int_Input*)w;
            app->trace_counter->step(atoi(tcs->value()));
        });
    tc_step->hide();
    trace_counter = new Fl_Spinner(w() - 150, 0, 100, height_of_menu);
    trace_counter->type(FL_INT_INPUT);
    trace_counter->callback([](Fl_Widget* w, void*)
        {
            Fl_Spinner* fct = (Fl_Spinner*)w;
            long long V = (long long)fct->value() - 1;
            app->trace_num = V;
            if (app->is_bin)app->change_profile_bin();
            else app->change_profile();
            app->trace_slider->value(V + 1);
        });
    trace_counter->hide();
    trace_slider = new Fl_Scrollbar(0, h() - height_of_scroll, w(), height_of_scroll);
    trace_slider->align(FL_ALIGN_LEFT);
    trace_slider->type(FL_HORIZONTAL);
    trace_slider->step(1);
    trace_slider->value(1);
    trace_slider->callback([](Fl_Widget* w, void*)
        {
            Fl_Scrollbar* fcs = (Fl_Scrollbar*)w;
            long long V = (long long)fcs->value() - 1;
            app->trace_num = V;
            if (!app->is_bin)
                app->change_profile();
            else
                app->change_profile_bin();
            app->trace_counter->value(V + 1);
        }, this);
    trace_slider->hide();
    ims = new Imagesc(0, height_of_menu, w(), h() - height_of_menu);
    open_button = new Fl_Button((Width - open_x) / 2, (Height - open_y) / 2, open_x, open_y);
    ims->hide();
    nan_box = new Fl_Box(w() - 275, 0, 25, height_of_menu, "!");
    nan_box->box(FL_NO_BOX);
    nan_box->labelcolor(FL_RED);
    nan_box->tooltip("数据中有nan值,已赋值为0");
    nan_box->labelfont(FL_TIMES_BOLD);
    nan_box->hide();
    open_button->label("打开文件\n或将文件拖入窗口");
    open_button->callback([](Fl_Widget*, void*)
        {
            app->fc->title("Open SEGY File");
            app->fc->type(Fl_Native_File_Chooser::BROWSE_FILE);
            switch (app->fc->show())
            {
            case -1:break;//Error
            case 1:fl_beep();break;//Cancel
            default:
                app->read_data(app->fc->filename());
                break;
            }
        });
    resizable(ims);
    end();
    ColorbarWin = new Fl_Window(660, 300, "Colorbar");
    ColorbarWin->begin();
    {
        const int cheight2 = 170, cheight3 = 220;
        Fl_Box* explaination = new Fl_Box(200, cheight2, 220, 120);
        explaination->box(FL_UP_BOX);
        explaination->align(FL_ALIGN_TOP | FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        explaination->label("双击色标点以更改颜色,右键以删除\n\n输入数据格式:\n第一行为色标点个数\n第二行开始为0-1之间的4列实数\n第一列为色标值,后三列为r,g,b值\n以逗号隔开");
        Fl_Button* swap_clr = new Fl_Button(50, cheight2, 100, 30, "reverse");
        Fl_Button* load_clr = new Fl_Button(50, cheight3, 100, 30, "load colormap");
        load_clr->callback(load_clr_cb);
        Fl_Button* save_clr = new Fl_Button(50, cheight3 + 40, 100, 30, "save colormap");
        save_clr->callback(save_clr_cb);
        chart = new Fl_Chart(30, 90, 600, 70);
        chart->type(FL_BAR_CHART);
        colormap_c = new Fl_Input_Choice(500, cheight2, 100, 30, "Colormap:");
        colormap_c->add("gray");
        colormap_c->add("rainbow");
        colormap_c->add("kwyr");
        colormap_c->add("rwb");
        colormap_c->value(3);
        colormap_c->callback([](Fl_Widget* w, void*)
            {
                Fl_Input_Choice* cmp = (Fl_Input_Choice*)w;
                char color_idx[16];
                strcpy(color_idx, cmp->value());
                colorbar->set_colormap(color_idx);
            });
        colorbar = new ColorBar(30, 60, 600, 20);
        swap_clr->callback([](Fl_Widget*, void*)
            {
                int tmp_x, i;
                for (i = 0;i < colorbar->c_num / 2;i++)
                {
                    tmp_c = colors[i];
                    colors[i] = colors[colorbar->c_num - i - 1];
                    colors[colorbar->c_num - i - 1] = tmp_c;
                    tmp_x = colors[i]->x();
                    colors[i]->position(colors[colorbar->c_num - i - 1]->x(), colors[i]->y());
                    colors[colorbar->c_num - i - 1]->position(tmp_x, colors[colorbar->c_num - i - 1]->y());
                }
                for (i = 0;i < colorbar->c_num;i++)colors[i]->redraw();
                colorbar->redraw();
                ims->redraw();
            });
        Fl_Button* end = new Fl_Button(570, 265, 80, 25, "ok");
        end->callback([](Fl_Widget*, void*) {app->ColorbarWin->hide();});
    }
    ColorbarWin->set_modal();
    ColorbarWin->redraw();
    ColorbarWin->end();
    ColorbarWin->hide();
}

void App::hide()
{
    Fl_Window::hide();
    delete app;
}

App::~App()
{
    close_them(true);
    free(seis);
    free(histo);
}

void App::close_them(bool all)
{
    delete scatter;scatter = NULL;
    delete cutter;cutter = NULL;
    delete converter;converter = NULL;
    delete CroppingWin;CroppingWin = NULL;
    delete hdrwin;hdrwin = NULL;
    delete property;property = NULL;
    delete rt;rt = NULL;
    // delete fkwin;fkwin=NULL;
    if (all)
        fclose(fpi);
}

int App::handle(int event)
{
    int ret = Fl_Window::handle(event);
    switch (event)
    {
    case FL_DND_ENTER:
    case FL_DND_DRAG:
    case FL_DND_RELEASE:ret = 1;break;
    case FL_PASTE:
        read_data(Fl::event_text());
        break;
    }
    return ret;
};

void App::resize(int x, int y, int Width, int Height)
{
    Fl_Window::resize(x, y, Width, Height);
    trace_s = traces < w() ? traces : w();
    int exs;
    if ((exs = trace_num + trace_s - traces) > 0)
        trace_num -= exs;
    if (seis)
    {
        update_slider();
        seis = (float*)realloc(seis, trace_s * samples * 4);
        change_profile();
    }
}

void App::update_figure()
{
    for (int cnt = 0;cnt < enhance_cnt;cnt++)
        for (int i = 0;i < trace_s * samples;i++)
            seis[i] = seis[i] < 0 ? -logf(1 - seis[i]) : logf(1 + seis[i]);

    ims->set_window_para();
    ims->redraw();
    ims->show();
    if (ColorbarWin->shown())
    {
        histo = (int*)realloc(histo, nbin * 4);//nbin==直方图区间个数
        if (ims->max_a - ims->min_a > FLT_MAX)return;
        for (int i = 0;i < trace_s * samples;i++)
        {
            int bin = ims->c_norm ? (int)(nbin * 0.5 * (seis[i] / (ims->max_abs_a + FLT_MIN) + 1)) :
                (int)(nbin * (seis[i] - ims->min_a) / (ims->max_a - ims->min_a + FLT_MIN));
            if (bin == nbin)bin--;
            histo[bin]++;
        }
        for (int i = 0;i < nbin;i++)chart->add(histo[i]);
    }
}

void App::read_data(const char* fname, bool utf_flag)
{
    ims->initial_ims();
    int i;
    is_bin = false;
    if (!first_read)
        app->close_them(true);
    if (utf_flag)fpi = fl_fopen(fname, "rb");
    else fpi = fopen(fname, "rb");
    if (fpi == NULL)
    {
        fl_alert("文件不存在!输入的路径为:%s\n可尝试在主窗口中打开", fname);
        return;
    }
    fseeko64(fpi, 0, SEEK_END);
    l = ftello64(fpi);
    if (l % 4 != 0)
    {
        fl_alert("文件错误,文件大小为%llu,不是4的倍数!", l);
        return;
    }
    sprintf(ld4_c, "%lld", l / 4);
    fseeko64(fpi, 0, 0);
    fread(hdr, 1, 3600, fpi);
    // fprintf(stderr,"%d,%d\n",hdr[3224],hdr[3225]);
    fseeko64(fpi, 0, 0);
    fmt = hdr[3224] + hdr[3225];
    if (fmt != 1 && fmt != 5 && fmt != 2)
    {//二进制文件
        is_bin = true;
        binpara = new BinPara(400, 200, "二进制参数");
        while (binpara->shown())Fl::wait();
        if (!binpara->pass)return;
    }
    else
    {
        if (hdr[3224] == 0)samples = hdr[3221] + hdr[3220] * 256;
        else if (hdr[3225] == 0)samples = hdr[3221] * 256 + hdr[3220];
        traces = (l - 3600) / (long long)(240 + 4 * samples);
        if (fmt == 5 && hdr[3225] == 0)fmk = 0;
        else if (fmt == 1 && hdr[3225] == 0)fmk = 1;
        else if (fmt == 5 && hdr[3224] == 0)fmk = 2;
        else if (fmt == 1 && hdr[3224] == 0)fmk = 3;
        else if (fmt == 2 && hdr[3225] == 0)fmk = 4;
        else if (fmt == 2 && hdr[3224] == 0)fmk = 5;
        fseeko64(fpi, 3600, 0);
    }// 此处已经确定好traces和samples数量
    trace_s = traces < w() ? traces : w();
    sprintf(traces_c, "%lld", traces);
    sprintf(samples_c, "%d", samples);
    // fprintf(stderr,"%lld,%d\n",traces,samples);
    seis = (float*)realloc(seis, sizeof(float) * trace_s * samples);
    for (i = 0;i < trace_s;i++)
    {
        if (!is_bin)fseeko64(fpi, 240, 1);
        fread(seis + samples * i, 4, samples, fpi);
    }
    format_correct(seis, trace_s * samples);
    detect_nan();

    update_figure();
    trace_num = 0;
    Fl_Menu_Item* item;
    if (first_read)
    {
        begin();
        where_mouse_box = new Fl_Input(w() - 250, 0, 100, height_of_menu);
        where_mouse_box->box(FL_THIN_DOWN_BOX);
        where_mouse_box->set_output();
        where_mouse_box->color(FL_BACKGROUND_COLOR);
        end();
        open_button->hide();
        item = (Fl_Menu_Item*)menus->find_item("Tools/Cutter-Separater");item->flags = 0;//激活按钮
        item = (Fl_Menu_Item*)menus->find_item("Tools/Format Converting");item->flags = 0;
        // item=(Fl_Menu_Item*)menus->find_item("Tools/F-k View");item->flags=0|FL_MENU_DIVIDER;
        item = (Fl_Menu_Item*)menus->find_item("Property");item->flags = 0;
        item = (Fl_Menu_Item*)menus->find_item("Tools/图像增强");item->flags = 0;
        item = (Fl_Menu_Item*)menus->find_item("Tools/图像减弱");item->flags = 0 | FL_MENU_DIVIDER;
        item = (Fl_Menu_Item*)menus->find_item("data?/ibm-le");item->flags = FL_MENU_RADIO;
        item = (Fl_Menu_Item*)menus->find_item("data?/ibm-be");item->flags = FL_MENU_RADIO;
        item = (Fl_Menu_Item*)menus->find_item("data?/ieee-le");item->flags = FL_MENU_RADIO;
        item = (Fl_Menu_Item*)menus->find_item("data?/ieee-be");item->flags = FL_MENU_RADIO;
        item = (Fl_Menu_Item*)menus->find_item("data?/int32-le");item->flags = FL_MENU_RADIO;
        item = (Fl_Menu_Item*)menus->find_item("data?/int32-be");item->flags = FL_MENU_RADIO | FL_MENU_DIVIDER;
        item = (Fl_Menu_Item*)menus->find_item("data?/zero-centered");item->flags = FL_MENU_TOGGLE;item->set();
        first_read = false;
    }
    item = (Fl_Menu_Item*)menus->find_item("Tools/Headers");item->flags = is_bin ? FL_MENU_INACTIVE : 0;
    item = (Fl_Menu_Item*)menus->find_item("Tools/Map view");item->flags = is_bin ? FL_MENU_INACTIVE : 0;
    switch (fmk)
    {
    case 0:
        item = (Fl_Menu_Item*)menus->find_item("data?/ieee-le");item->set();
        break;
    case 1:
        item = (Fl_Menu_Item*)menus->find_item("data?/ibm-le");item->set();
        break;
    case 2:
        item = (Fl_Menu_Item*)menus->find_item("data?/ieee-be");item->set();
        break;
    case 3:
        item = (Fl_Menu_Item*)menus->find_item("data?/ibm-be");item->set();
        break;
    case 4:
        item = (Fl_Menu_Item*)menus->find_item("data?/int32-le");item->set();
        break;
    case 5:
        item = (Fl_Menu_Item*)menus->find_item("data?/int32-be");item->set();
        break;
    }
    strcpy(filename, fname);
    label(fname);
    findnames(app->filename, &(app->filenames));
    update_slider();
    redraw();
}

void App::update_slider()
{
    if (traces > w()) // 文件太大则制作滚动条
    {
        ims->size(w(), h() - height_of_menu - height_of_scroll);
        trace_slider->bounds(1, traces - trace_s + 1);
        trace_counter->range(1, app->traces);
        tc_step->show();
        trace_slider->show();
        trace_counter->show();
    }
    else
    {
        ims->size(w(), h() - height_of_menu);
        trace_slider->hide();
        trace_counter->hide();
    }

}

void App::detect_nan()
{
    bool nan_flag = false;
    for (int i = 0;i < trace_s * samples;i++)
    {
        if (isnan(seis[i]) || isinf(seis[i]))
        {
            nan_flag = true;
            seis[i] = 0;
        }
    }
    if (nan_flag)
        nan_box->show();
    else
        nan_box->hide();
}

void App::ieee2ibm(float* seis_b, int length)//公式已经考虑了字节序，无需再用swap_bytes转换
{
    for (int tr = 0;tr < length;tr++)
    {
        if ((seis_b[tr]) != 0)
        {
            char sgn;
            unsigned char E, * p;
            p = (unsigned char*)&seis_b[tr];
            sgn = 1 - 2 * (p[0] >> 7);
            E = p[0] << 1;
            E >>= 1;
            seis_b[tr] = sgn * ((*(p + 1) << 16) + (*(p + 2) << 8) + *(p + 3)) * pow_table[E];
        }
    }
}

BinPara::BinPara(int W, int H, const char* title) :Fl_Window(W, H, title)
{
    begin();
    default_icon(ico);
    guess = new Fl_Button(300, 10, 80, 30, "猜测");
    guess->tooltip("开发中>_<敬请期待");
    ok = new Fl_Button(300, 50, 80, 30, "ok");
    ok->callback([](Fl_Widget*, void*)
        {
            app->traces = atoi(binpara->traces_input->value());
            app->samples = atoi(binpara->samples_input->value());
            binpara->pass = true;
            Fl_Menu_Item* item = (Fl_Menu_Item*)app->menus->find_item("Tools/Textual Header Editor");item->flags = FL_MENU_INACTIVE;
            item = (Fl_Menu_Item*)app->menus->find_item("Tools/Headers");item->flags = FL_MENU_INACTIVE;
            Fl::delete_widget(binpara);
        });
    notes = new Fl_Box(0, 80, 400, 120);
    sprintf(notes_c, "文件大小为%llu字节\n默认一个采样点占用4字节则共%llu个采样点\n因此满足 道数×采样点数=%llu", app->l, app->l / 4, app->l / 4);
    notes->label(notes_c);
    traces_input = new Fl_Int_Input(100, 10, 100, 30, "道数");
    samples_input = new Fl_Int_Input(100, 50, 100, 30, "采样点数");
    traces_input->value("1");
    samples_input->value(app->ld4_c);
    traces_input->when(FL_WHEN_CHANGED);
    samples_input->when(FL_WHEN_CHANGED);
    traces_input->callback([](Fl_Widget*, void*)
        {
            if (0 == strcmp(binpara->traces_input->value(), "\0"))return;
            long long V;
            char buf[32];
            V = atoi(binpara->traces_input->value());
            if (V<1 || V>app->l / 4)
            {
                binpara->traces_input->value("1");
                binpara->samples_input->value(app->ld4_c);
                return;
            }
            sprintf(buf, "%lld", app->l / (4 * V));
            binpara->samples_input->value(buf);
        });
    samples_input->callback([](Fl_Widget*, void*)
        {
            if (0 == strcmp(binpara->samples_input->value(), "\0"))return;
            long long V;
            char buf[32];
            V = atoi(binpara->samples_input->value());
            if (V<1 || V>app->l / 4)
            {
                binpara->samples_input->value("1");
                binpara->traces_input->value(app->ld4_c);
                return;
            }
            sprintf(buf, "%lld", app->l / (4 * V));
            binpara->traces_input->value(buf);
        });
    end();
    show();
}

void HeaderTable::draw_cell(TableContext context, int ROW, int COL, int X, int Y, int W, int H)
{
    static char s[32];
    switch (context)
    {
    case CONTEXT_STARTPAGE:
        fl_font(FL_HELVETICA, 12);
        return;
    case CONTEXT_ROW_HEADER:
        if (ROW == 0)
        {
            DrawHeader("Trace:", X, Y, W, H);
            return;
        }
        sprintf(s, "b%d", ROW);
        DrawHeader(s, X, Y, W, H);
        return;
    case CONTEXT_CELL:
        if (ROW == 0)
        {
            sprintf(s, "%lld", trace_num + COL + 1);
            DrawHeader(s, X, Y, W, H);
            return;
        }
        if (selected_R == 0 || selected_R > 241 - bytes_num)return;
        sprintf(s, "%d", hdrbytes_s[COL * 240 + ROW - 1]);
        fl_push_clip(X, Y, W, H);
        if (bytes_num == 4)
            fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, (ROW == selected_R || ROW == selected_R + 1 || ROW == selected_R + 2 || ROW == selected_R + 3) ? FL_YELLOW : FL_WHITE);
        else //if bytes_num==2
            fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, (ROW == selected_R || ROW == selected_R + 1) ? FL_YELLOW : FL_WHITE);
        fl_color(FL_GRAY0);fl_draw(s, X, Y, W, H, FL_ALIGN_CENTER);
        fl_pop_clip();
        return;
    default:
        return;
    }
}

void HeaderTable::DrawHeader(const char* s, int X, int Y, int W, int H)
{
    fl_push_clip(X, Y, W, H);
    fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, row_header_color());
    fl_color(FL_BLACK);
    fl_draw(s, X, Y, W, H, FL_ALIGN_CENTER);
    fl_pop_clip();
}

HeaderTable::HeaderTable(int X, int Y, int W, int H, const char* title) :Fl_Table_Row(X, Y, W, H, title)
{
    hdrbytes_s = (unsigned char*)malloc(trace_s_h * 240);
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
    fseeko64(app->fpi, 3600 + trace_num * (240 + 4 * app->samples), 0);
    for (int i = 0;i < trace_s_h;i++)
    {
        fread(hdrbytes_s + i * 240, 1, 240, app->fpi);
        fseeko64(app->fpi, app->samples * 4, 1);
    }
}

HeaderTable::~HeaderTable()
{
    free(hdrbytes_s);
}

int HeaderTable::handle(int event)
{
    int ret = Fl_Table_Row::handle(event);
    if (event == FL_PUSH)
    {
        for (selected_R = 0;selected_R < rows();selected_R++)
            if (row_selected(selected_R))
                break;
        redraw();
        rt->redraw();
    }
    return ret;
}

VHeaderTable::VHeaderTable(int X, int Y, int W, int H, const char* title) :Fl_Table(X, Y, W, H, title)
{
    fseeko64(app->fpi, 3200, 0);
    fread(vhdrbytes, 1, 400, app->fpi);
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

void VHeaderTable::draw_cell(TableContext context, int ROW, int COL, int X, int Y, int W, int H)
{
    static char s[32];
    switch (context)
    {
    case CONTEXT_STARTPAGE:
        fl_font(FL_HELVETICA, 12);
        return;
    case CONTEXT_ROW_HEADER:
        sprintf(s, "%d:", ROW + 3201);
        fl_push_clip(X, Y, W, H);
        fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, row_header_color());
        fl_color(FL_BLACK);
        fl_draw(s, X, Y, W, H, FL_ALIGN_CENTER);
        fl_pop_clip();
        return;
    case CONTEXT_CELL:
        sprintf(s, "%d", vhdrbytes[ROW]);
        fl_push_clip(X, Y, W, H);
        fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, is_selected(ROW, COL) ? FL_YELLOW : FL_WHITE);
        fl_color(FL_GRAY0);fl_draw(s, X, Y, W, H, FL_ALIGN_CENTER);
        fl_pop_clip();
        return;
    default:
        return;
    }
}

ResultTable::ResultTable(int X, int Y, int W, int H, const char* title) :Fl_Table(X, Y, W, H, title)
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

void ResultTable::draw_cell(TableContext context, int ROW, int COL, int X, int Y, int W, int H)
{
    static char s[32];
    switch (context)
    {
    case CONTEXT_STARTPAGE:
        fl_font(FL_HELVETICA, 12);
        return;
    case CONTEXT_ROW_HEADER:
        fl_push_clip(X, Y, W, H);
        fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, row_header_color());
        fl_color(FL_BLACK);
        fl_draw("Value:", X, Y, W, H, FL_ALIGN_CENTER);
        fl_pop_clip();
        return;
    case CONTEXT_CELL:
        fl_push_clip(X, Y, W, H);
        sprintf(s, "%d", b2i(COL));
        fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, FL_WHITE);
        fl_color(FL_GRAY0);
        fl_draw(s, X, Y, W, H, FL_ALIGN_CENTER);
        fl_pop_clip();
    default:
        return;
    }
}

int ResultTable::b2i(int C)
{
    int tnum;
    unsigned char* tbytes, tmp_b;
    tbytes = (unsigned char*)&tnum;
    tbytes[0] = headertable->hdrbytes_s[C * 240 + headertable->selected_R - 1];
    tbytes[1] = headertable->hdrbytes_s[C * 240 + headertable->selected_R];
    tbytes[2] = headertable->hdrbytes_s[C * 240 + headertable->selected_R + 1];
    tbytes[3] = headertable->hdrbytes_s[C * 240 + headertable->selected_R + 2];
    if (!app->is_le)
    {
        tmp_b = tbytes[0];tbytes[0] = tbytes[3];tbytes[3] = tmp_b;
        tmp_b = tbytes[1];tbytes[1] = tbytes[2];tbytes[2] = tmp_b;
    }
    return tnum;
}

MapViewWin::MapViewWin(int W, int H, const char* title) :Fl_Window(W, H, title)
{
    default_icon(ico);
    text = new Fl_Box(0, 0, 500, 30, "请输入对应信息所在道头的起始位置,可在Header工具中查询");
    valueX = new Fl_Int_Input(30, 30, 40, 25, "X:");
    valueY = new Fl_Int_Input(30, 60, 40, 25, "Y:");
    ok = new Fl_Button(80, 30, 50, 25, "确认");
    app->endian_menu1 = new Fl_Menu_Bar(80, 60, 200, 25);
    app->endian_menu1->add("big-endian", 0, hdrwin->endian_cb, 0, FL_MENU_RADIO);
    app->endian_menu1->add("little-endian", 0, hdrwin->endian_cb, 0, FL_MENU_RADIO);
    app->update_endian_menu(app->endian_menu1);
    scatter = new Scatter(10, 90, 480, 400);
    ok->callback([](Fl_Widget*, void*)
        {
            int Xbyte, Ybyte;
            scatter->xmax = scatter->ymax = 0;
            scatter->xmin = scatter->ymin = INT_MAX;
            Xbyte = atoi(mapview->valueX->value());
            Ybyte = atoi(mapview->valueY->value());
            fseeko64(app->fpi, 3600, 0);
            ret_flag = false;
            if (Xbyte > Ybyte)scatter->findmvalues(Ybyte, Xbyte, scatter->ymax, scatter->ymin, scatter->ys, scatter->xmax, scatter->xmin, scatter->xs);
            else scatter->findmvalues(Xbyte, Ybyte, scatter->xmax, scatter->xmin, scatter->xs, scatter->ymax, scatter->ymin, scatter->ys);
            if (ret_flag)return;
            sprintf(scatter->ax_buf[0], "%d", scatter->ymax);
            sprintf(scatter->ax_buf[1], "%d", scatter->ymin);
            sprintf(scatter->ax_buf[2], "%d", scatter->xmin);
            sprintf(scatter->ax_buf[3], "%d", scatter->xmax);
            scatter->redraw();
            scatter->isvalid = true;
        });
    end();
    show();
}

void findnames(char* filename, manynames* outfile)
{
    strcpy(outfile->path, "");
    strcpy(outfile->path_slash, "");
    strcpy(outfile->ext, "");
    int i;
    i = strlen(filename) - 1;
    while (i >= 0 && filename[i] != '\\' && filename[i] != '/')i--;
    if (i >= 0)
    {
        strcpy(outfile->basename, filename + i + 1);
        strncpy(outfile->path, filename, i);
        strncpy(outfile->path_slash, filename, i + 1);
        outfile->path[i] = '\0';
        if (outfile->path[i - 1] == '\\' || outfile->path[i - 1] == '/')
            outfile->path[i - 1] = '\0';
        outfile->path_slash[i + 1] = '\0';
    }
    else
        strcpy(outfile->basename, filename);
    i = strlen(outfile->basename) - 1;
    while (i >= 0 && outfile->basename[i] != '.')i--;
    if (i >= 0)
    {
        strncpy(outfile->base, outfile->basename, i);
        outfile->base[i] = '\0';
        strcpy(outfile->ext, &outfile->basename[i + 1]);
    }
    else
        strcpy(outfile->base, outfile->basename);
}

void expand_right_menu(int evx, int evy, char* num)
{
    Fl_Menu_Button right_menu1(Fl::event_x(), Fl::event_y(), 0, 0, num);
    right_menu1.type(Fl_Menu_Button::POPUP3);
    right_menu1.box(FL_UP_BOX);
    right_menu1.add("色标", 0, [](Fl_Widget*, void*)
        {
            app->ColorbarWin->show();
        }, 0);
    right_menu1.add("该列数据_", 0, [](Fl_Widget*, void*)
        {

        }, 0);
    right_menu1.popup();
}

void check_progress(Progress* progress, long long tr, long long total)
{
    ret_flag = false;
    char p100_c[36];
    if (tr % (long long)(total / 100.0) == 0)
    {
        sprintf(p100_c, "进度:%d%%,单击以停止...", p100);
        progress->copy_label(p100_c);
        progress->value(p100++);
        Fl::check();
    }
    if (progress->stop)ret_flag = true;
}

int main(int argc, char** argv)
{
    app = new App(100, 100, 600, 600, "Open a segy file");
    app->show(1, argv);
    if (argc > 1)
        app->read_data(argv[1], false);
    return Fl::run();
}
