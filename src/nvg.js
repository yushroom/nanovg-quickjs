import * as nvg from "nanovg";

const ICON_SEARCH = 0x1F50D;
const ICON_CIRCLED_CROSS = 0x2716;
const ICON_CHEVRON_RIGHT = 0xE75E;
const ICON_CHECK = 0x2713;
const ICON_LOGIN = 0xE740;
const ICON_TRASH = 0xE729;

function drawWindow(title, x, y, w, h)
{
    const cornerRadius = 3.0;

    nvg.Save();
//  nvgClearState(vg);

    // Window
    nvg.BeginPath();
    nvg.RoundedRect(x,y, w,h, cornerRadius);
    nvg.FillColor(nvgRGBA(28,30,34,192));
//  nvgFillColor(nvgRGBA(0,0,0,128));
    nvg.Fill();

    // Drop shadow
    const shadowPaint = nvg.BoxGradient(x,y+2, w,h, cornerRadius*2, 10, nvgRGBA(0,0,0,128), nvgRGBA(0,0,0,0));
    nvg.BeginPath();
    nvg.Rect(x-10,y-10, w+20,h+30);
    nvg.RoundedRect(x,y, w,h, cornerRadius);
    nvg.PathWinding(nvg.HOLE);
    nvg.FillPaint(shadowPaint);
    nvg.Fill();

    // Header
    const headerPaint = nvg.LinearGradient(x,y,x,y+15, nvgRGBA(255,255,255,8), nvgRGBA(0,0,0,16));
    nvg.BeginPath();
    nvg.RoundedRect(x+1,y+1, w-2,30, cornerRadius-1);
    nvg.FillPaint(headerPaint);
    nvg.Fill();
    nvg.BeginPath();
    nvg.MoveTo(x+0.5, y+0.5+30);
    nvg.LineTo(x+0.5+w-1, y+0.5+30);
    nvg.StrokeColor(nvgRGBA(0,0,0,32));
    nvg.Stroke();

    nvg.FontSize(18.0);
    nvg.FontFace("sans-bold");
    nvg.TextAlign(nvg.ALIGN_CENTER|nvg.ALIGN_MIDDLE);

    nvg.FontBlur(2);
    nvg.FillColor(nvgRGBA(0,0,0,128));
    nvg.Text(x+w/2,y+16+1, title);

    nvg.FontBlur(0);
    nvg.FillColor(nvgRGBA(220,220,220,160));
    nvg.Text(x+w/2,y+16, title);

    nvg.Restore();
}

function drawSearchBox(text, x, y, w, h)
{
    const cornerRadius = h/2-1;

    // Edit
    const bg = nvg.BoxGradient(x,y+1.5, w,h, h/2,5, nvgRGBA(0,0,0,16), nvgRGBA(0,0,0,92));
    nvg.BeginPath();
    nvg.RoundedRect(x,y, w,h, cornerRadius);
    nvg.FillPaint(bg);
    nvg.Fill();

/*  nvgBeginPath(vg);
    nvgRoundedRect(x+0.5f,y+0.5f, w-1,h-1, cornerRadius-0.5f);
    nvgStrokeColor(nvgRGBA(0,0,0,48));
    nvgStroke(vg);*/

    nvg.FontSize(h*1.3);
    nvg.FontFace("icons");
    nvg.FillColor(nvgRGBA(255,255,255,64));
    nvg.TextAlign(nvg.ALIGN_CENTER|nvg.ALIGN_MIDDLE);
    nvg.Text(x+h*0.55, y+h*0.55, String.fromCodePoint(ICON_SEARCH));

    nvg.FontSize(20.0);
    nvg.FontFace("sans");
    nvg.FillColor(nvgRGBA(255,255,255,32));

    nvg.TextAlign(nvg.ALIGN_LEFT|nvg.ALIGN_MIDDLE);
    nvg.Text(x+h*1.05,y+h*0.5,text);

    nvg.FontSize(h*1.3);
    nvg.FontFace("icons");
    nvg.FillColor(nvgRGBA(255,255,255,32));
    nvg.TextAlign(nvg.ALIGN_CENTER|nvg.ALIGN_MIDDLE);
    nvg.Text(x+w-h*0.55, y+h*0.55, String.fromCodePoint(ICON_CIRCLED_CROSS));
}

function drawDropDown(text, x, y, w, h)
{
    const cornerRadius = 4.0;

    const bg = nvg.LinearGradient(x,y,x,y+h, nvgRGBA(255,255,255,16), nvgRGBA(0,0,0,16));
    nvg.BeginPath();
    nvg.RoundedRect(x+1,y+1, w-2,h-2, cornerRadius-1);
    nvg.FillPaint(bg);
    nvg.Fill();

    nvg.BeginPath();
    nvg.RoundedRect(x+0.5,y+0.5, w-1,h-1, cornerRadius-0.5);
    nvg.StrokeColor(nvgRGBA(0,0,0,48));
    nvg.Stroke();

    nvg.FontSize(20.0);
    nvg.FontFace("sans");
    nvg.FillColor(nvgRGBA(255,255,255,160));
    nvg.TextAlign(nvg.ALIGN_LEFT|nvg.ALIGN_MIDDLE);
    nvg.Text(x+h*0.3,y+h*0.5,text);

    nvg.FontSize(h*1.3);
    nvg.FontFace("icons");
    nvg.FillColor(nvgRGBA(255,255,255,64));
    nvg.TextAlign(nvg.ALIGN_CENTER|nvg.ALIGN_MIDDLE);
    nvg.Text(x+w-h*0.5, y+h*0.5, String.fromCodePoint(ICON_CHEVRON_RIGHT));
}

function drawLabel(text, x, y, w, h) {
    nvg.FontSize(18.0);
    nvg.FontFace("sans");
    nvg.FillColor(nvgRGBA(255,255,255,128));

    nvg.TextAlign(nvg.ALIGN_LEFT | nvg.ALIGN_MIDDLE);
    nvg.Text(x, y+h*0.5, text);
}

function drawEditBoxBase(x, y, w, h) {
    const bg = nvg.BoxGradient(x+1,y+1+1.5, w-2,h-2, 3,4, nvgRGBA(255,255,255,32), nvgRGBA(32,32,32,32));
    nvg.BeginPath();
    nvg.RoundedRect(x+1, y+1, w-2, h-2, 4-1);
    nvg.FillPaint(bg);
    nvg.Fill();

    nvg.BeginPath();
    nvg.RoundedRect(x+0.5, y+0.5, w-1,h-1, 4-0.5);
    nvg.StrokeColor(nvgRGBA(0,0,0,48));
    nvg.Stroke();
}

function drawEditBox(text, x, y, w, h)
{
    drawEditBoxBase(x, y, w, h);
    nvg.FontSize(20.0);
    nvg.FontFace("sans");
    nvg.FillColor(nvgRGBA(255,255,255,64));
    nvg.TextAlign(nvg.ALIGN_LEFT | nvg.ALIGN_MIDDLE);
    nvg.Text(x+h*0.3, y+h*0.5, text);
}

function drawEditBoxNum(text, units, x, y, w, h)
{
    drawEditBoxBase(x,y, w,h);

    const uw = nvg.TextBounds(0,0, units);

    nvg.FontSize(18.0);
    nvg.FontFace("sans");
    nvg.FillColor(nvgRGBA(255,255,255,64));
    nvg.TextAlign(nvg.ALIGN_RIGHT|nvg.ALIGN_MIDDLE);
    nvg.Text(x+w-h*0.3,y+h*0.5,units);

    nvg.FontSize(20.0);
    nvg.FontFace("sans");
    nvg.FillColor(nvgRGBA(255,255,255,128));
    nvg.TextAlign(nvg.ALIGN_RIGHT|nvg.ALIGN_MIDDLE);
    nvg.Text(x+w-uw-h*0.5,y+h*0.5,text,);
}


function drawCheckBox(text, x, y, w, h)
{
    nvg.FontSize(18.0);
    nvg.FontFace("sans");
    nvg.FillColor(nvgRGBA(255,255,255,160));

    nvg.TextAlign(nvg.ALIGN_LEFT|nvg.ALIGN_MIDDLE);
    nvg.Text(x+28, y+h*0.5, text);

    let bg = nvg.BoxGradient(x+1, y+(h*0.5)-9+1, 18, 18, 3, 3, nvgRGBA(0,0,0,32), nvgRGBA(0,0,0,92));
    nvg.BeginPath();
    nvg.RoundedRect(x+1,y+(h*0.5)-9, 18,18, 3);
    nvg.FillPaint(bg);
    nvg.Fill();

    nvg.FontSize(40);
    nvg.FontFace("icons");
    nvg.FillColor(nvgRGBA(255,255,255,128));
    nvg.TextAlign(nvg.ALIGN_CENTER|nvg.ALIGN_MIDDLE);
    nvg.Text(x+9+2, y+h*0.5, String.fromCodePoint(ICON_CHECK));
}

function nvgRGBA(r, g, b, a) {
    return {r:r/255.0, g:g/255.0, b:b/255.0, a:a/255.0};
}

function isBlack(col) {
    return col.r === 0.0 && col.g === 0.0 && col.b === 0.0 && col.a === 0.0;
}

function drawButton(preicon, text, x, y, w, h, col) {
    const cornerRadius = 4.0;
    let iw = 0;

    let bg = nvg.LinearGradient(x,y,x,y+h, nvgRGBA(255,255,255,isBlack(col)?16:32), nvgRGBA(0,0,0,isBlack(col)?16:32));
    nvg.BeginPath();
    nvg.RoundedRect(x+1,y+1, w-2,h-2, cornerRadius-1);
    if (!isBlack(col)) {
        nvg.FillColor(col);
        nvg.Fill();
    }
    nvg.FillPaint(bg);
    nvg.Fill();

    nvg.BeginPath();
    nvg.RoundedRect(x+0.5,y+0.5, w-1,h-1, cornerRadius-0.5);
    nvg.StrokeColor(nvgRGBA(0,0,0,48));
    nvg.Stroke();

    nvg.FontSize(20.0);
    nvg.FontFace("sans-bold");
    const tw = nvg.TextBounds(0,0, text);
    if (preicon != 0) {
        nvg.FontSize(h*1.3);
        nvg.FontFace("icons");
        iw = nvg.TextBounds(0,0, String.fromCodePoint(preicon));
        iw += h*0.15;
    }

    if (preicon != 0) {
        nvg.FontSize(h*1.3);
        nvg.FontFace("icons");
        nvg.FillColor(nvgRGBA(255,255,255,96));
        nvg.TextAlign(nvg.ALIGN_LEFT|nvg.ALIGN_MIDDLE);
        nvg.Text(x+w*0.5-tw*0.5-iw*0.75, y+h*0.5, String.fromCodePoint(preicon));
    }

    nvg.FontSize(20.0);
    nvg.FontFace("sans-bold");
    nvg.TextAlign(nvg.ALIGN_LEFT|nvg.ALIGN_MIDDLE);
    nvg.FillColor(nvgRGBA(0,0,0,160));
    nvg.Text(x+w*0.5-tw*0.5+iw*0.25,y+h*0.5-1,text);
    nvg.FillColor(nvgRGBA(255,255,255,160));
    nvg.Text(x+w*0.5-tw*0.5+iw*0.25,y+h*0.5,text);
}

function drawSlider(pos, x, y, w, h)
{
    const cy = y+(h*0.5);
    const kr = (h*0.25);

    nvg.Save();
//  nvgClearState(vg);

    // Slot
    let bg = nvg.BoxGradient(x,cy-2+1, w,4, 2,2, nvgRGBA(0,0,0,32), nvgRGBA(0,0,0,128));
    nvg.BeginPath();
    nvg.RoundedRect(x,cy-2, w,4, 2);
    nvg.FillPaint(bg);
    nvg.Fill();

    // Knob Shadow
    bg = nvg.RadialGradient(x+(pos*w),cy+1, kr-3,kr+3, nvgRGBA(0,0,0,64), nvgRGBA(0,0,0,0));
    nvg.BeginPath();
    nvg.Rect(x+(pos*w)-kr-5,cy-kr-5,kr*2+5+5,kr*2+5+5+3);
    nvg.Circle(x+(pos*w),cy, kr);
    nvg.PathWinding(nvg.HOLE);
    nvg.FillPaint(bg);
    nvg.Fill();

    // Knob
    const knob = nvg.LinearGradient(x,cy-kr,x,cy+kr, nvgRGBA(255,255,255,16), nvgRGBA(0,0,0,16));
    nvg.BeginPath();
    nvg.Circle(x+(pos*w),cy, kr-1);
    nvg.FillColor(nvgRGBA(40,43,48,255));
    nvg.Fill();
    nvg.FillPaint(knob);
    nvg.Fill();

    nvg.BeginPath();
    nvg.Circle(x+(pos*w),cy, kr-0.5);
    nvg.StrokeColor(nvgRGBA(0,0,0,92));
    nvg.Stroke();

    nvg.Restore();
}


function renderDemo() {
    nvg.Save();
    let x = 600;
    let y = 50;

    // Widgets
    drawWindow("Widgets `n Stuff", x, y, 300, 400);
    x += 10;
    y += 45;
    drawSearchBox("Search", x,y,280,25);
    y += 40;
    drawDropDown("Effects", x,y,280,28);
    y += 45;

    // Form
    drawLabel("Login", x, y, 280, 20);
    y += 25;
    drawEditBox("Email", x, y, 280, 28);
    y += 35;
    drawEditBox("Password", x, y, 280, 28);
    y += 38;
    drawCheckBox("Remember me", x,y, 140,28);
    drawButton(ICON_LOGIN, "Sign in", x+138, y, 140, 28, nvgRGBA(0,96,128,255));
    y += 45;

    // Slider
    drawLabel("Diameter", x, y, 280, 20);
    y += 25;
    drawEditBoxNum("123.00", "px", x+180,y, 100,28);
    drawSlider(0.4, x,y, 170,28);
    y += 55;

    drawButton(ICON_TRASH, "Delete", x, y, 160, 28, nvgRGBA(128,16,8,255));
    drawButton(0, "Cancel", x+170, y, 110, 28, nvgRGBA(0,0,0,0));
    nvg.Restore();
}

renderDemo();