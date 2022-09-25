// https://codereview.stackexchange.com/questions/63427/simple-key-value-store-in-c

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "kvs.h"

//////////////////////////////////////////////////////////////

                        //FUNCOES KVS//

//////////////////////////////////////////////////////////////

static const size_t kvs_pair_size = sizeof(KVSpair);

static const size_t kvs_store_size = sizeof(KVSstore);

static int kvs_sort_compare(const void *a, const void *b) {
    const KVSpair *pairA = a;
    const KVSpair *pairB = b;
    if (pairA->key > pairB->key) {
        return -1;
    }
    if (pairA->key < pairB->key) {
        return 1;
    }
    return 0;
}

static int kvs_search_compare(const void *key, const void *element) {
    const KVSpair *pair = element;
    if (key > pair->key) {
        return -1;
    }
    if (key < pair->key) {
        return 1;
    }
    return 0;
}

static KVSpair *kvs_get_pair(KVSstore *store, const void *key) {
    if ((!store) || (!store->pairs)) {
        return NULL;
    }
    return bsearch(key, store->pairs, store->length, kvs_pair_size,
                    kvs_search_compare);
}

static void kvs_sort_pairs(KVSstore *store) {
    if ((!store) || (!store->pairs)) {
        return;
    }
    qsort(store->pairs, store->length, kvs_pair_size, kvs_sort_compare);
}

static void kvs_resize_pairs(KVSstore *store) {
    if (!store) {
        return;
    }
    store->pairs = realloc(store->pairs, kvs_pair_size * store->length);
}

static void kvs_create_pair(KVSstore *store, const void *key, void *value) {
    KVSpair *pair;
    if (!store) {
        return;
    }
    ++store->length;
    kvs_resize_pairs(store);
    pair = &store->pairs[store->length - 1];
    pair->key = key;
    pair->value = value;
    kvs_sort_pairs(store);
}

static void kvs_remove_pair(KVSstore *store, KVSpair *pair) {
    if ((!store) || (!pair)) {
        return;
    }
    pair->key = NULL;
    kvs_sort_pairs(store);
    --store->length;
    kvs_resize_pairs(store);
}

KVSstore *kvs_create(void) {
    KVSstore *store = malloc(kvs_store_size);
    store->pairs = NULL;
    store->length = 0;
    return store;
}

void kvs_destroy(KVSstore *store) {
    if (!store) {
        return;
    }
    if (store->pairs) {
        free(store->pairs);
    }
    free(store);
}

void kvs_put(KVSstore *store, const void *key, void *value) {
    KVSpair *pair = kvs_get_pair(store, key);
    if (pair) {
        if (value) {
            pair->value = value;
        } else {
            kvs_remove_pair(store, pair);
        }
    } else if (value) {
        kvs_create_pair(store, key, value);
    }
}

void *kvs_get(KVSstore *store, const void *key) {
    KVSpair *pair = kvs_get_pair(store, key);
    return pair ? pair->value : NULL;
}

//////////////////////////////////////////////////////////////

                        //FUNCOES LUA//

//////////////////////////////////////////////////////////////

/* function declaration */
void addDATA (lua_State* L, const char* chave, const char* valor, KVSstore *store);
void addCPF (lua_State* L, const char* chave, const char* valor, KVSstore *store);
void error (lua_State *L, const char *fmt, ...);

/* call a function `validarCPF' defined in Lua */
void addCPF (lua_State* L, const char* chave, const char* valor, KVSstore *store) {
  const char* w;
  const char* z;

  //printf("chave: %s , valor: %s", chave, valor);
  lua_getglobal(L, "validarCPF");  /* function to be called */
  lua_pushstring (L, chave);   /* push 1nd argument */
  lua_pushstring (L, valor);   /* push 2nd argument */

    /* do the call (2 arguments, 1 result) */
  if (lua_pcall(L, 2, 1, 0) != 0)
    error(L, "error running function `validarCPF': %s",
              lua_tostring(L, -1));

  w = lua_tostring(L, -2);
  z = lua_tostring(L, -1);

  lua_pop(L, 1);  /* pop returned value */
  printf("CPF: %s válido!\n", z);

  kvs_put(store, w, z);
}

/* call a function `validarISO8601' defined in Lua */
void addDATA (lua_State* L, const char* chave, const char* valor, KVSstore *store) {
  const char* z;
  const char* w;

  //printf("chave: %s , valor: %s", chave, valor);
  lua_getglobal(L, "validarISO8601");  /* function to be called */
  lua_pushstring (L, chave);   /* push 1nd argument */
  lua_pushstring (L, valor);   /* push 2nd argument */

    /* do the call (2 arguments, 1 result) */
  if (lua_pcall(L, 2, 1, 0) != 0)
    error(L, "error running function `validarISO8601': %s",
              lua_tostring(L, -1));

  w = lua_tostring(L, -2);
  z = lua_tostring(L, -1);

  lua_pop(L, 1);  /* pop returned value */
  printf("DATA: %s válida!\n", z);

  kvs_put(store, w, z);
}

void error (lua_State *L, const char *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  vfprintf(stderr, fmt, argp);
  va_end(argp);
  lua_close(L);
  exit(EXIT_FAILURE);
}


int main(int argc, const char* argv[]){

  lua_State* L = luaL_newstate();
  luaL_openlibs(L);

  // carrega o arquivo e executa suas as ações
  luaL_dofile(L, "script.lua");

  int opcao;
  const char* cpf = (char*) malloc(17 *sizeof(char));
  const char* data = (char*) malloc(17 *sizeof(char));
  const char* valor = (char*) malloc(17 *sizeof(char));

  KVSstore *store = kvs_create();

  

  printf("                        Bem vindo(a)!");
  printf("                                      EP 1 - BD em Lua + C!  ");
  printf("\n\n");

  do { 
    
    printf("\nEscolha as opções a seguir:\n\n ");
    printf("1. ADD\n");
    printf("2. Instruções de preenchimento\n");
    printf("3. Sair\n");
    
    scanf("%d", &opcao);
    system("cls || clear");
    
    switch(opcao)
    {
      case 1:
        
        printf("Digite um CPF para adicionar: ");
        scanf("%s", cpf);
        addCPF(L,"CPF", cpf, store);
      
        printf("Digite uma DATA para adicionar: ");
        scanf("%s", data);
        addDATA(L,"DATA", data, store);

        break;
    
      case 2:
          printf("Para efetivação de registro de CPF insira no padrão (111.111.111-11)\n");
          printf("\nPara efetivação de registro de DATA insira no padrão ISO8601 (2022-10-10)\n");
          break;
      case 3:
          exit(EXIT_FAILURE);
    
      default:
          printf("Digite uma opcao valida\n");
      
    }
  }while(opcao!=3);

  lua_close(L);

  return 0;
}


/*
referencias:

https://stackoverflow.com/questions/10158450/how-to-check-if-matching-text-is-found-in-a-string-in-lua
https://www.lua.org/manual/5.1/pt/manual.html
http://www.troubleshooters.com/codecorn/lua/lua_c_calls_lua.htm
https://www.lua.org/pil/24.2.1.html
https://stackoverflow.com/questions/45941230/regex-as-lua-pattern
http://lua-users.org/wiki/PatternsTutorial

*/