
char* jerry_get_utf8_string(jerry_value_t v) {
  jerry_size_t size = jerry_get_utf8_string_size(v);
  char* str = (char*)TKMEM_ALLOC(size + 1);
  return_value_if_fail(str != NULL, NULL);

  jerry_string_to_utf8_char_buffer(v, (jerry_char_t*)str, size);
  str[size] = '\0';

  return str;
}

wchar_t* jerry_get_wstring(jerry_value_t v) {
  wchar_t* wstr = NULL;
  jerry_size_t size = jerry_get_utf8_string_size(v);
  char* str = (char*)TKMEM_ALLOC(size + 1);

  return_value_if_fail(str != NULL, NULL);

  wstr = (wchar_t*)TKMEM_ALLOC(sizeof(wchar_t) * (size + 1));
  jerry_string_to_utf8_char_buffer(v, (jerry_char_t*)str, size);
  str[size] = '\0';

  utf8_to_utf16(str, wstr, size + 1);
  TKMEM_FREE(str);

  return wstr;
}

jerry_value_t jerry_create_string_from_wstring(const wchar_t* wstr) {
  int32_t size = 0;
  char* str = NULL;
  jerry_value_t ret;
  if (wstr == NULL) {
    return jerry_create_null();
  }

  size = wcslen(wstr) * 4 + 1;
  str = (char*)TKMEM_ALLOC(size);
  if (str == NULL) {
    return jerry_create_null();
  }

  utf8_from_utf16(wstr, str, size);
  ret = jerry_create_string_from_utf8((jerry_char_t*)str);
  TKMEM_FREE(str);

  return ret;
}

void* jerry_get_pointer(jerry_value_t v, const char* type) {
  char str[64];
  void* p = NULL;
  jerry_value_t cls_type = jerry_create_string_from_utf8((jerry_char_t*)"class_type");
  jerry_value_t cls_type_value = jerry_get_property(v, cls_type);

  if (jerry_value_is_null(v) || jerry_value_is_undefined(v)) {
    return NULL;
  }

  memset(str, 0x00, sizeof(str));
  jerry_string_to_utf8_char_buffer(cls_type_value, (jerry_char_t*)str, sizeof(str));

  jerry_get_object_native_pointer(v, &p, NULL);

  return p;
}

jerry_value_t jerry_create_pointer(const void* ptr, const char* type) {
  if (ptr == NULL) {
    return jerry_create_null();
  } else {
    jerry_value_t obj = jerry_create_object();
    jerry_value_t cls_type = jerry_create_string_from_utf8((jerry_char_t*)"class_type");
    jerry_value_t cls_type_value = jerry_create_string_from_utf8((jerry_char_t*)type);

    jerry_set_property(obj, cls_type, cls_type_value);
    jerry_set_object_native_pointer(obj, (void*)ptr, NULL);

    return obj;
  }
}

static ret_t call_on_event(void* ctx, event_t* e) {
  jerry_value_t res;
  jerry_value_t args[1];
  jerry_value_t func = (jerry_value_t)((int32_t*)ctx - (int32_t*)NULL);
  jerry_value_t this_value = jerry_create_undefined();

  args[0] = jerry_create_pointer(e, "event_t");
  res = jerry_call_function(func, this_value, args, 1);

  jerry_release_value(args[0]);
  jerry_release_value(this_value);

  return (ret_t)jerry_get_number_value(res);
}

jerry_value_t wrap_widget_on(const jerry_value_t func_obj_val, const jerry_value_t this_p,
                             const jerry_value_t args_p[], const jerry_length_t args_cnt) {
  int32_t ret = 0;
  return_value_if_fail(args_cnt >= 2, jerry_create_undefined());

  if (args_cnt >= 2) {
    widget_t* widget = (widget_t*)jerry_get_pointer(args_p[0], "widget_t*");
    event_type_t type = (event_type_t)jerry_get_number_value(args_p[1]);
    jerry_value_t on_event = jerry_acquire_value(args_p[2]);

    void* ctx = (int32_t*)NULL + (int32_t)on_event;
    ret = (int32_t)widget_on(widget, type, call_on_event, ctx);
  }

  return jerry_create_number(ret);
}

jerry_value_t wrap_widget_off(const jerry_value_t func_obj_val, const jerry_value_t this_p,
                              const jerry_value_t args_p[], const jerry_length_t args_cnt) {
  ret_t ret = RET_FAIL;
  return_value_if_fail(args_cnt >= 1, jerry_create_undefined());

  if (args_cnt >= 1) {
    widget_t* widget = (widget_t*)jerry_get_pointer(args_p[0], "widget_t*");
    int32_t id = (int32_t)jerry_get_number_value(args_p[1]);
    emitter_item_t* item = emitter_find(widget->emitter, id);

    if (item) {
      jerry_value_t func = (char*)(item->ctx) - (char*)NULL;
      ret = (ret_t)widget_off(widget, id);
      jerry_release_value(func);
    }
  }

  return jerry_create_number(ret);
}

jerry_value_t wrap_tklocale_on(const jerry_value_t func_obj_val, const jerry_value_t this_p,
                               const jerry_value_t args_p[], const jerry_length_t args_cnt) {
  int32_t ret = 0;
  return_value_if_fail(args_cnt >= 2, jerry_create_undefined());

  if (args_cnt >= 2) {
    tklocale_t* tklocale = (tklocale_t*)jerry_get_pointer(args_p[0], "tklocale_t*");
    event_type_t type = (event_type_t)jerry_get_number_value(args_p[1]);
    jerry_value_t on_event = jerry_acquire_value(args_p[2]);

    void* ctx = (int32_t*)NULL + (int32_t)on_event;
    ret = (uint32_t)tklocale_on(tklocale, type, call_on_event, ctx);
  }

  return jerry_create_number(ret);
}

jerry_value_t wrap_tklocale_off(const jerry_value_t func_obj_val, const jerry_value_t this_p,
                                const jerry_value_t args_p[], const jerry_length_t args_cnt) {
  ret_t ret = RET_FAIL;
  return_value_if_fail(args_cnt >= 1, jerry_create_undefined());

  if (args_cnt >= 1) {
    tklocale_t* tklocale = (tklocale_t*)jerry_get_pointer(args_p[0], "tklocale_t*");
    uint32_t id = (uint32_t)jerry_get_number_value(args_p[1]);
    emitter_item_t* item = emitter_find(tklocale->emitter, id);

    if (item) {
      jerry_value_t func = (char*)(item->ctx) - (char*)NULL;
      ret = (ret_t)tklocale_off(tklocale, id);
      jerry_release_value(func);
    }
  }

  return jerry_create_number(ret);
}

static ret_t call_on_timer(const timer_info_t* timer) {
  jerry_value_t res;
  jerry_value_t args[1];
  jerry_value_t this_value = jerry_create_undefined();
  jerry_value_t func = (jerry_value_t)((int32_t*)timer->ctx - (int32_t*)NULL);

  args[0] = jerry_create_undefined();
  res = jerry_call_function(func, this_value, args, 1);

  jerry_release_value(args[0]);
  jerry_release_value(this_value);

  return (ret_t)jerry_get_number_value(res);
}

jerry_value_t wrap_timer_add(const jerry_value_t func_obj_val, const jerry_value_t this_p,
                             const jerry_value_t args_p[], const jerry_length_t args_cnt) {
  int32_t ret = 0;
  return_value_if_fail(args_cnt >= 2, jerry_create_undefined());

  if (args_cnt >= 2) {
    jerry_value_t on_timer = jerry_acquire_value(args_p[0]);
    uint32_t duration_ms = (uint32_t)jerry_get_number_value(args_p[1]);
    void* ctx = (int32_t*)NULL + (int32_t)on_timer;

    ret = (uint32_t)timer_add(call_on_timer, ctx, duration_ms);
  }

  return jerry_create_number(ret);
}

jerry_value_t wrap_timer_remove(const jerry_value_t func_obj_val, const jerry_value_t this_p,
                                const jerry_value_t args_p[], const jerry_length_t args_cnt) {
  ret_t ret = RET_FAIL;
  return_value_if_fail(args_cnt >= 1, jerry_create_undefined());

  if (args_cnt >= 1) {
    uint32_t timer_id = (uint32_t)jerry_get_number_value(args_p[0]);
    const timer_info_t* timer = timer_find(timer_id);
    if (timer != NULL) {
      uint32_t func = (char*)(timer->ctx) - (char*)NULL;
      ret = (ret_t)timer_remove(timer_id);
      jerry_release_value(func);
    }
  }

  return jerry_create_number(ret);
}

static ret_t call_on_idle(const idle_info_t* idle) {
  jerry_value_t res;
  jerry_value_t args[1];
  jerry_value_t this_value = jerry_create_undefined();
  jerry_value_t func = (jerry_value_t)((int32_t*)idle->ctx - (int32_t*)NULL);

  args[0] = jerry_create_undefined();
  res = jerry_call_function(func, this_value, args, 1);

  jerry_release_value(args[0]);
  jerry_release_value(this_value);

  return (ret_t)jerry_get_number_value(res);
}

jerry_value_t wrap_idle_add(const jerry_value_t func_obj_val, const jerry_value_t this_p,
                            const jerry_value_t args_p[], const jerry_length_t args_cnt) {
  int32_t ret = 0;
  return_value_if_fail(args_cnt >= 2, jerry_create_undefined());

  if (args_cnt >= 1) {
    jerry_value_t on_idle = jerry_acquire_value(args_p[0]);
    void* ctx = (int32_t*)NULL + (int32_t)on_idle;

    ret = (uint32_t)idle_add(call_on_idle, ctx);
  }

  return jerry_create_number(ret);
}

jerry_value_t wrap_idle_remove(const jerry_value_t func_obj_val, const jerry_value_t this_p,
                               const jerry_value_t args_p[], const jerry_length_t args_cnt) {
  ret_t ret = RET_FAIL;
  return_value_if_fail(args_cnt >= 1, jerry_create_undefined());

  if (args_cnt >= 1) {
    uint32_t idle_id = (uint32_t)jerry_get_number_value(args_p[0]);
    const idle_info_t* idle = idle_find(idle_id);
    uint32_t func = (char*)(idle->ctx) - (char*)NULL;

    ret = (ret_t)idle_remove(idle_id);
    jerry_release_value(func);
  }

  return jerry_create_number(ret);
}

static ret_t call_visit(void* ctx, void* data) {
  jerry_value_t res;
  jerry_value_t args[1];
  jerry_value_t func = (jerry_value_t)((int32_t*)ctx - (int32_t*)NULL);
  jerry_value_t this_value = jerry_create_undefined();

  args[0] = jerry_create_pointer(data, "widget_t*");
  res = jerry_call_function(func, this_value, args, 1);

  jerry_release_value(args[0]);
  jerry_release_value(this_value);

  return (ret_t)jerry_get_number_value(res);
}

jerry_value_t wrap_widget_foreach(const jerry_value_t func_obj_val, const jerry_value_t this_p,
                                  const jerry_value_t args_p[], const jerry_length_t args_cnt) {
  ret_t ret = RET_OK;

  return_value_if_fail(args_cnt >= 2, jerry_create_undefined());
  if (args_cnt >= 2) i {
      widget_t* widget = (widget_t*)jerry_get_pointer(args_p[0], "widget_t*");
      jerry_value_t func = args_p[1];
      void* ctx = (int32_t*)NULL + (int32_t)func;

      ret = (ret_t)widget_foreach(widget, call_visit, ctx);
    }

  return jerry_create_number(ret);
}
