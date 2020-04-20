#include "jszlpriv.h"

static struct jszlnode *get_value_byidx
(struct jszlnode *value, unsigned idx)
{
  if(idx >= value->count) return 0; //err: Index too high
  if( !(value = value->child) ) return 0; //err: no values
  while(idx--){
    assert(value);
    value = value->next;
  }
  return value;
}

static struct jszlnode *get_value_byname(
  struct jszlcontext *handle
 ,struct jszlnode *value
 ,unsigned long hash
 ,unsigned len
){
  struct atom *atom = 0;
  void * l_atomTable = g_AtomTable;
  //hash = djb2(name, len);
  atom = atom_find(l_atomTable, ATOM_TABLE_SIZE, hash, len);
  assert(atom);
  if(!atom) return 0;
  for(value = value->child; value; value = value->next)
    if(value->name == atom) break;
  return value; 
}



// $ = absolute root
// # = relative root
int resolve_root(
  struct jszlcontext *pctx
 ,struct jszlnode **ppnode
 ,const symbol
){
  if(!pctx || !ppnode || !symbol) return JSZLE_BAD_PARAM;
  if(!pctx->RootNS) return JszlE_NoRoot;

  switch(symbol){
    case '$':
      *ppnode = pctx->RootNS;
      break;
    case '#':
      *ppnode = pctx->CurrentNS;
      break;
    default:
      return JSZLE_BAD_PATH;
  }

  return JszlE_None; 
}

int get_node_byname(
  struct jszlnode *pnode
 ,struct jszlnode **ppnode
 ,const char *name
){
  unsigned n;
  unsigned long hash;
  struct atom *atom = 0;
  void * l_atomTable = g_AtomTable;

  n = is_valid_key(name, global_seed, &hash);
  if(!n)    return 0;
  atom = atom_find(l_atomTable, ATOM_TABLE_SIZE, hash, n);
  if(!atom) return 0;

  for(pnode = pnode->child; pnode; pnode = pnode->next)
    if(pnode->name == atom) break;
  if(!pnode) return 0;
  *ppnode = pnode;
  return n;
}
