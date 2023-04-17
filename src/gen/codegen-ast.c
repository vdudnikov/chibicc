#include "chibicc.h"
#include "gen/jemi.h"

#define JEMI_NODES_MAX 30000
static jemi_node_t node_pool[JEMI_NODES_MAX];

jemi_node_t* jemi_string_or_null(const char *str) {
  if (str == NULL)
    return jemi_null();
  return jemi_string(str);
}

int deep = 0;

static jemi_node_t* serialize_obj(Obj *root) {
  if (root == NULL || deep == 1) {
    return jemi_null();
  }

  jemi_node_t *root_node = jemi_array(NULL);

  for (Obj *obj = root; obj; obj = obj->next) {
    jemi_node_t *obj_node = jemi_object(
      jemi_string("name"), jemi_string(obj->name),
      // ty
      // tok
      jemi_string("is_local"), jemi_bool(obj->is_local),
      jemi_string("align"), jemi_integer(obj->align),
      jemi_string("offset"), jemi_integer(obj->offset),
      jemi_string("is_function"), jemi_bool(obj->is_function),
      jemi_string("is_definition"), jemi_bool(obj->is_definition),
      jemi_string("is_static"), jemi_bool(obj->is_static),
      jemi_string("is_tentative"), jemi_bool(obj->is_tentative),
      jemi_string("is_tls"), jemi_bool(obj->is_tls),
      // init_data
      // rel
      jemi_string("is_inline"), jemi_bool(obj->is_inline),
      jemi_string("params"), serialize_obj(obj->params),
      // body
      // locals
      // va_area
      // alloca_bottom
      jemi_string("stack_size"), jemi_integer(obj->stack_size),
      jemi_string("is_live"), jemi_bool(obj->is_live),
      jemi_string("is_root"), jemi_bool(obj->is_root),
      // refs
      NULL
    );

    jemi_object_append(root_node, jemi_list(obj_node, NULL));
  }

  return root_node;
}

void codegen_target_ast(Obj *prog, FILE *out) {
  jemi_init(node_pool, JEMI_NODES_MAX);
  jemi_node_t *root_node = serialize_obj(prog);
  jemi_emit(root_node, (void (*)(char))putchar, NULL);
  putchar('\n');
}
