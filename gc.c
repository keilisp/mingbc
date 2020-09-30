#include <stdio.h>
#include <stdlib.h>

// VM's max stack size
#define STACK_MAX 256
// Number of objects required for a first GC call
#define GC_THRESHOLD 10

/* Prototype of garbage */
typedef enum {
  OBJ_INT, // Integer
  OBJ_PAIR // Pair of smth
} ObjType;

/* "Garbage" object implemented as linked list */
typedef struct sObj {
  // Next object in the list
  struct sObj *next;
  unsigned char marked; // Mark bit
  ObjType type;
  union {
    // OBJ_INT
    int value;
    // OBJ_PAIR
    struct {
      struct sObj *head;
      struct sObj *tail;
    };
  };
} Obj;

/* Basic Virtual Machine */
typedef struct {
  int numObjs;           // Total count of allocated objects
  int maxObjs;           // Number of objects required to trigger a GC call
  Obj *headObj;          // The first object in the list
  Obj *stack[STACK_MAX]; // Stack array
  int stackSize;         // Current stack size
} VM;

/* New VM */
VM *newVM() {
  VM *vm = malloc(sizeof(VM));
  vm->headObj = NULL;
  vm->stackSize = 0;

  vm->numObjs = 0;
  vm->maxObjs = GC_THRESHOLD;
  return vm;
}

/* VM's stack manipulation */

void push(VM *vm, Obj *val) {
  if (vm->stackSize >= STACK_MAX) {
    printf("Stack overflowed!");
    exit(1);
  }
  vm->stack[vm->stackSize++] = val;
};

Obj *pop(VM *vm) {
  if (vm->stackSize < 0) {
    printf("Stack underflow!");
    exit(1);
  }
  return vm->stack[--vm->stackSize];
}

void mark(Obj *object) {
  // Check if already marked
  if (object->marked)
    return;

  // Set mark bit to 1
  object->marked = 1;

  // Recursively mark references
  if (object->type == OBJ_PAIR) {
    mark(object->head);
    mark(object->tail);
  }
}

void markAll(VM *vm) {
  for (int i = 0; i < vm->stackSize; i++) {
    mark(vm->stack[i]);
  }
}

/* Sweep */
// Sweep through ad delete the unmarked objects

void sweep(VM *vm) {
  Obj **object = &vm->headObj;
  while (*object) {
    if (!(*object)->marked) {
      // Remove from the list and free
      Obj *unreached = *object;

      *object = unreached->next;
      // Decrement counter of objects
      vm->numObjs--;
      free(unreached);
    } else {
      // Unmark it and move to the next object
      (*object)->marked = 0;
      object = &(*object)->next;
    }
  }
}

// Collect "Garbage"
void gc(VM *vm) {
  int numObjs = vm->numObjs;

  markAll(vm);
  sweep(vm);

  // Update the max number of objects for gc
  vm->maxObjs = vm->numObjs * 2;
  printf("Collected %d objects, %d remaining.\n", numObjs - vm->numObjs,
         vm->numObjs);
}

// Create new Obj
Obj *newObj(VM *vm, ObjType type) {
  if (vm->numObjs == vm->maxObjs)
    gc(vm);
  Obj *object = malloc(sizeof(Obj));
  object->type = type;
  object->marked = 0;

  // Insert it into the list
  object->next = vm->headObj;
  vm->headObj = object;
  // Increment counter of objects
  vm->numObjs++;
  return object;
}

// Push int onto VM's stack
void pushInt(VM *vm, int intValue) {
  Obj *object = newObj(vm, OBJ_INT);
  object->value = intValue;
  push(vm, object);
}

// Push pair onto VM's stack
Obj *pushPair(VM *vm) {
  Obj *object = newObj(vm, OBJ_PAIR);
  object->tail = pop(vm);
  object->head = pop(vm);

  push(vm, object);
  return object;
}

void objPrint(Obj *object) {
  switch (object->type) {
  case OBJ_INT:
    printf("%d", object->value);
    break;
  case OBJ_PAIR:
    printf("(");
    objPrint(object->head);
    printf(",");
    objPrint(object->tail);
    printf(")");
  }
}

// Free Virtual Machine
void freeVM(VM *vm) {
  vm->stackSize = 0;
  gc(vm);
  free(vm);
}

int main() {
  VM *vm = newVM();
  pushInt(vm, 1);
  pushInt(vm, 2);
  pushPair(vm);
  pushInt(vm, 3);
  pushInt(vm, 4);
  pushPair(vm);
  pushPair(vm);
  pushInt(vm, 5);
  pushInt(vm, 7);

  gc(vm);
  freeVM(vm);
}
