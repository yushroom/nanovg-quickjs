#include "nanovg_qjs.h"
#include <nanovg.h>
#include <assert.h>
#include <cutils.h>
#include <quickjs.h>
#include <quickjs-libc.h>

int JS_ToFloat32(JSContext *ctx, float *pres, JSValueConst val)
{
	double f;
	int ret = JS_ToFloat64(ctx, &f, val);
	if (ret == 0)
		*pres = (float)f;
	return ret;
}

static int eval_buf(JSContext *ctx, const void *buf, int buf_len,
                    const char *filename, int eval_flags)
{
    JSValue val;
    int ret;

    if ((eval_flags & JS_EVAL_TYPE_MASK) == JS_EVAL_TYPE_MODULE) {
        /* for the modules, we compile then run to be able to set
           import.meta */
        val = JS_Eval(ctx, buf, buf_len, filename,
                      eval_flags | JS_EVAL_FLAG_COMPILE_ONLY);
        if (!JS_IsException(val)) {
            js_module_set_import_meta(ctx, val, TRUE, TRUE);
            val = JS_EvalFunction(ctx, val);
        }
    } else {
        val = JS_Eval(ctx, buf, buf_len, filename, eval_flags);
    }
    if (JS_IsException(val)) {
        js_std_dump_error(ctx);
        ret = -1;
    } else {
        ret = 0;
    }
    JS_FreeValue(ctx, val);
    return ret;
}

static int eval_file(JSContext *ctx, const char *filename, int module)
{
    uint8_t *buf;
    int ret, eval_flags;
    size_t buf_len;
    
    buf = js_load_file(ctx, &buf_len, filename);
    if (!buf) {
        perror(filename);
        exit(1);
    }

    if (module < 0) {
        module = (has_suffix(filename, ".mjs") ||
                  JS_DetectModule((const char *)buf, buf_len));
    }
    if (module)
        eval_flags = JS_EVAL_TYPE_MODULE;
    else
        eval_flags = JS_EVAL_TYPE_GLOBAL;
    ret = eval_buf(ctx, buf, buf_len, filename, eval_flags);
    js_free(ctx, buf);
    return ret;
}


static JSRuntime *g_RT = NULL;
static JSContext *g_Context = NULL;


void CallMethod(JSValueConst obj, const char* methodName)
{
    assert(JS_IsObject(obj));
    JSValue method = JS_GetPropertyStr(g_Context, obj, methodName);
    assert(!JS_IsException(method) && !JS_IsNull(method) && !JS_IsUndefined(method));
    {
        JSValue ret;
        ret = JS_Call(g_Context, method, obj, 0, NULL);
        JS_FreeValue(g_Context, method);
        JS_FreeValue(g_Context, ret);
    }
}


//void qjs_tick()
//{
//	js_std_tick(g_Context);
//}


NVGcontext *g_NVGcontext = NULL;

static JSClassID js_nanovg_paint_class_id;

static void js_nanovg_paint_finalizer(JSRuntime *rt, JSValue val)
{
	NVGpaint *p = JS_GetOpaque(val, js_nanovg_paint_class_id);
	if (p) {
		js_free_rt(rt, p);
	}
}

static JSValue js_nanovg_paint_ctor(JSContext *ctx,
									JSValueConst new_target,
									int argc, JSValueConst *argv)
{
	NVGpaint *p;
	JSValue obj = JS_UNDEFINED;
	JSValue proto;
	
	p = js_mallocz(ctx, sizeof(*p));
	if (!p)
		return JS_EXCEPTION;
	
	proto = JS_GetProperty(ctx, new_target, "prototype");
	if (JS_IsException(proto))
		goto fail;
	obj = JS_NewObjectProtoClass(ctx, proto, js_nanovg_paint_class_id);
	JS_FreeValue(ctx, proto);
	if (JS_IsException(obj))
		goto fail;
	JS_SetOpaque(obj, p);
	return obj;
fail:
	js_free(ctx, p);
	JS_FreeValue(ctx, obj);
	return JS_EXCEPTION;
}

static JSClassDef js_nanovg_paint_class = {
	"Paint",
	.finalizer = js_nanovg_paint_finalizer,
};


static JSValue js_nanovg_wrap(JSContext *ctx, void *s, JSClassID classID)
{
	JSValue obj = JS_NewObjectClass(ctx, classID);
	if (JS_IsException(obj))
		return obj;
	JS_SetOpaque(obj, s);
	return obj;
}

int GetFloat32PropertyStr(JSContext *ctx, JSValueConst this_obj, const char *prop, float *pres)
{
	JSValue p = JS_GetPropertyStr(ctx, this_obj, prop);
	int ret = JS_ToFloat32(ctx, pres, p);
	JS_FreeValue(ctx, p);
	return ret;
}

NVGcolor ConvertToNVGcolor(JSContext *ctx, JSValueConst this_obj)
{
	NVGcolor color;
	GetFloat32PropertyStr(ctx, this_obj, "r", &color.r);
	GetFloat32PropertyStr(ctx, this_obj, "g", &color.g);
	GetFloat32PropertyStr(ctx, this_obj, "b", &color.b);
	GetFloat32PropertyStr(ctx, this_obj, "a", &color.a);
	return color;
}


#define FUNC(fn) \
static JSValue js_nanovg_##fn(JSContext *ctx, JSValueConst this_value, \
							  int argc, JSValueConst *argv) \

FUNC(Save)
{
	nvgSave(g_NVGcontext);
	return JS_UNDEFINED;
}

FUNC(Restore)
{
	nvgReset(g_NVGcontext);
	return JS_UNDEFINED;
}

FUNC(Rect)
{
	assert(argc == 4);
	float x, y, w, h;
	JS_ToFloat32(ctx, &x, argv[0]);
	JS_ToFloat32(ctx, &y, argv[1]);
	JS_ToFloat32(ctx, &w, argv[2]);
	JS_ToFloat32(ctx, &h, argv[3]);
	nvgRect(g_NVGcontext, x, y, w, h);
	return JS_UNDEFINED;
}

FUNC(Circle)
{
	assert(argc == 3);
	float cx, cy, r;
	JS_ToFloat32(ctx, &cx, argv[0]);
	JS_ToFloat32(ctx, &cy, argv[1]);
	JS_ToFloat32(ctx, &r, argv[2]);
	nvgCircle(g_NVGcontext, cx, cy, r);
	return JS_UNDEFINED;
}

FUNC(PathWinding)
{
	assert(argc == 1);
	int dir;
	JS_ToInt32(ctx, &dir, argv[0]);
	nvgPathWinding(g_NVGcontext, dir);
	return JS_UNDEFINED;
}

FUNC(MoveTo)
{
	assert(argc == 2);
	float x, y;
	JS_ToFloat32(ctx, &x, argv[0]);
	JS_ToFloat32(ctx, &y, argv[1]);
	nvgMoveTo(g_NVGcontext, x, y);
	return JS_UNDEFINED;
}

FUNC(LineTo)
{
	float x, y;
	JS_ToFloat32(ctx, &x, argv[0]);
	JS_ToFloat32(ctx, &y, argv[1]);
	nvgLineTo(g_NVGcontext, x, y);
	return JS_UNDEFINED;
}

FUNC(FontBlur)
{
	float blur;
	JS_ToFloat32(ctx, &blur, argv[0]);
	nvgFontBlur(g_NVGcontext, blur);
	return JS_UNDEFINED;
}

FUNC(BeginPath)
{
	nvgBeginPath(g_NVGcontext);
	return JS_UNDEFINED;
}

FUNC(RoundedRect)
{
	float x, y, w, h, r;
	assert(argc == 5);
	JS_ToFloat32(ctx, &x, argv[0]);
	JS_ToFloat32(ctx, &y, argv[1]);
	JS_ToFloat32(ctx, &w, argv[2]);
	JS_ToFloat32(ctx, &h, argv[3]);
	JS_ToFloat32(ctx, &r, argv[4]);
	nvgRoundedRect(g_NVGcontext, x, y, w, h, r);
	return JS_UNDEFINED;
}

FUNC(FillPaint)
{
	assert(argc == 1);
	NVGpaint *paint = JS_GetOpaque(argv[0], js_nanovg_paint_class_id);
	nvgFillPaint(g_NVGcontext, *paint);
	return JS_UNDEFINED;
}

FUNC(Fill)
{
	nvgFill(g_NVGcontext);
	return JS_UNDEFINED;
}

FUNC(StrokeColor)
{
	assert(argc == 1);
	NVGcolor color = ConvertToNVGcolor(ctx, argv[0]);
	nvgStrokeColor(g_NVGcontext, color);
	return JS_UNDEFINED;
}

FUNC(Stroke)
{
	nvgStroke(g_NVGcontext);
	return JS_UNDEFINED;
}

FUNC(FillColor)
{
	assert(argc == 1);
	NVGcolor color = ConvertToNVGcolor(ctx, argv[0]);
	nvgFillColor(g_NVGcontext, color);
	return JS_UNDEFINED;
}

FUNC(LinearGradient)
{
	float sx, sy, ex, ey;
	NVGcolor icol, ocol;
	assert(argc == 6);
	JS_ToFloat32(ctx, &sx, argv[0]);
	JS_ToFloat32(ctx, &sy, argv[1]);
	JS_ToFloat32(ctx, &ex, argv[2]);
	JS_ToFloat32(ctx, &ey, argv[3]);
	icol = ConvertToNVGcolor(ctx, argv[4]);
	ocol = ConvertToNVGcolor(ctx, argv[5]);
	NVGpaint paint = nvgLinearGradient(g_NVGcontext, sx, sy, ex, ey, icol, ocol);
	NVGpaint *p = js_mallocz(ctx, sizeof(NVGpaint));
	*p = paint;
	return js_nanovg_wrap(ctx, p, js_nanovg_paint_class_id);
}

FUNC(BoxGradient)
{
	float x, y, w, h, r, f;
	NVGcolor icol, ocol;
	assert(argc == 8);
	JS_ToFloat32(ctx, &x, argv[0]);
	JS_ToFloat32(ctx, &y, argv[1]);
	JS_ToFloat32(ctx, &w, argv[2]);
	JS_ToFloat32(ctx, &h, argv[3]);
	JS_ToFloat32(ctx, &r, argv[4]);
	JS_ToFloat32(ctx, &f, argv[5]);
	icol = ConvertToNVGcolor(ctx, argv[6]);
	ocol = ConvertToNVGcolor(ctx, argv[7]);

	NVGpaint paint = nvgBoxGradient(g_NVGcontext, x, y, w, h, r, f, icol, ocol);
	NVGpaint *p = js_mallocz(ctx, sizeof(NVGpaint));
	*p = paint;
	return js_nanovg_wrap(ctx, p, js_nanovg_paint_class_id);
}

FUNC(RadialGradient)
{
	float cx, cy, inr, outr;
	NVGcolor icol, ocol;
	assert(argc == 6);
	JS_ToFloat32(ctx, &cx, argv[0]);
	JS_ToFloat32(ctx, &cy, argv[1]);
	JS_ToFloat32(ctx, &inr, argv[2]);
	JS_ToFloat32(ctx, &outr, argv[3]);
	icol = ConvertToNVGcolor(ctx, argv[4]);
	ocol = ConvertToNVGcolor(ctx, argv[5]);
	
	NVGpaint paint = nvgRadialGradient(g_NVGcontext, cx, cy, inr, outr, icol, ocol);
	NVGpaint *p = js_mallocz(ctx, sizeof(NVGpaint));
	*p = paint;
	return js_nanovg_wrap(ctx, p, js_nanovg_paint_class_id);
}

FUNC(TextBounds)
{
	float x, y;
	const char *str = NULL;
	assert(argc == 3);
	JS_ToFloat32(ctx, &x, argv[0]);
	JS_ToFloat32(ctx, &y, argv[1]);
	str = JS_ToCString(ctx, argv[2]);
	float ret = nvgTextBounds(g_NVGcontext, x, y, str, NULL, NULL);
	return JS_NewFloat64(ctx, ret);
}

FUNC(FontSize)
{
	assert(argc == 1);
	double size;
	if (!JS_ToFloat64(ctx, &size, argv[0]))
		nvgFontSize(g_NVGcontext, (float)size);
	return JS_UNDEFINED;
}

FUNC(FontFace)
{
	assert(argc == 1);
	const char* str = JS_ToCString(ctx, argv[0]);
	nvgFontFace(g_NVGcontext, str);
	JS_FreeCString(ctx, str);
	return JS_UNDEFINED;
}

FUNC(TextAlign)
{
	assert(argc == 1);
	int align;
	if (!JS_ToInt32(ctx, &align, argv[0]))
		nvgTextAlign(g_NVGcontext, align);
	return JS_UNDEFINED;
}

FUNC(Text)
{
	int x, y;
	const char *str = NULL;
	assert(argc == 3);
	if (JS_ToInt32(ctx, &x, argv[0]))
		goto fail;
	if (JS_ToInt32(ctx, &y, argv[1]))
		goto fail;
	str = JS_ToCString(ctx, argv[2]);
	float ret = nvgText(g_NVGcontext, x, y, str, NULL);
	return JS_NewFloat64(ctx, ret);
fail:
	JS_FreeCString(ctx, str);
	return JS_EXCEPTION;
}

#define _JS_CFUNC_DEF(fn, length) JS_CFUNC_DEF(#fn, length, js_nanovg_##fn)

static const JSCFunctionListEntry js_nanovg_funcs[] = {
	_JS_CFUNC_DEF(Save, 0),
	_JS_CFUNC_DEF(Restore, 0),
	_JS_CFUNC_DEF(BeginPath, 0),
	_JS_CFUNC_DEF(RoundedRect, 5),
	_JS_CFUNC_DEF(FillPaint, 1),
	_JS_CFUNC_DEF(Fill, 0),
	_JS_CFUNC_DEF(StrokeColor, 4),
	_JS_CFUNC_DEF(Stroke, 0),
	_JS_CFUNC_DEF(FillColor, 4),
	_JS_CFUNC_DEF(LinearGradient, 6),
	_JS_CFUNC_DEF(BoxGradient, 8),
	_JS_CFUNC_DEF(RadialGradient, 6),
	_JS_CFUNC_DEF(TextBounds, 3),
	_JS_CFUNC_DEF(Rect, 4),
	_JS_CFUNC_DEF(Circle, 3),
	_JS_CFUNC_DEF(PathWinding, 1),
	_JS_CFUNC_DEF(MoveTo, 2),
	_JS_CFUNC_DEF(LineTo, 2),
	_JS_CFUNC_DEF(FontBlur, 1),
	_JS_CFUNC_DEF(FontSize, 1),
	_JS_CFUNC_DEF(FontFace, 1),
	_JS_CFUNC_DEF(TextAlign, 1),
	_JS_CFUNC_DEF(Text, 3),
	JS_PROP_INT32_DEF("ALIGN_LEFT", NVG_ALIGN_LEFT, JS_PROP_CONFIGURABLE),
	JS_PROP_INT32_DEF("ALIGN_CENTER", NVG_ALIGN_CENTER, JS_PROP_CONFIGURABLE),
	JS_PROP_INT32_DEF("ALIGN_RIGHT", NVG_ALIGN_RIGHT, JS_PROP_CONFIGURABLE),
	JS_PROP_INT32_DEF("ALIGN_TOP", NVG_ALIGN_TOP, JS_PROP_CONFIGURABLE),
	JS_PROP_INT32_DEF("ALIGN_MIDDLE", NVG_ALIGN_MIDDLE, JS_PROP_CONFIGURABLE),
	JS_PROP_INT32_DEF("ALIGN_BOTTOM", NVG_ALIGN_BOTTOM, JS_PROP_CONFIGURABLE),
	JS_PROP_INT32_DEF("HOLE", NVG_HOLE, JS_PROP_CONFIGURABLE),
};

static int js_nanovg_init(JSContext *ctx, JSModuleDef *m)
{
	JSValue paint_proto, paint_class;
	
	JS_NewClassID(&js_nanovg_paint_class_id);
	JS_NewClass(JS_GetRuntime(ctx), js_nanovg_paint_class_id, &js_nanovg_paint_class);
	
	paint_proto = JS_NewObject(ctx);
	JS_SetClassProto(ctx, js_nanovg_paint_class_id, paint_proto);
	paint_class = JS_NewCFunction2(ctx, js_nanovg_paint_ctor, "Paint", 0, JS_CFUNC_constructor, 0);
	JS_SetConstructor(ctx, paint_class, paint_proto);
	
	JS_SetModuleExport(ctx, m, "Paint", paint_class);
    JS_SetModuleExportList(ctx, m, js_nanovg_funcs, countof(js_nanovg_funcs));
	return 0;
}

JSModuleDef *js_init_module_nanovg(JSContext *ctx, const char *module_name)
{
    JSModuleDef *m;
    m = JS_NewCModule(ctx, module_name, js_nanovg_init);
    if (!m)
        return NULL;
	JS_AddModuleExport(ctx, m, "Paint");
    JS_AddModuleExportList(ctx, m, js_nanovg_funcs, countof(js_nanovg_funcs));
    return m;
}


void RunFile(const char* path)
{
    if (g_Context == NULL)
    {
        g_RT = JS_NewRuntime();
        g_Context = JS_NewContext(g_RT);
        js_std_add_helpers(g_Context, 0, NULL);
        js_init_module_os(g_Context, "os");
        js_init_module_nanovg(g_Context, "nanovg");
    }

    int module = 1;
    eval_file(g_Context, path, module);
}
