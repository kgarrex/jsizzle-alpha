
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


int query_engine(
  struct jszlnode *pnode
 ,struct jszlnode **ppnode
 ,const char *path)
{
  const char *loc;
  unsigned n, type, subtype;
  unsigned short idx;


  loc = path;

begin_loop:

  if(*loc == '\0'){
    *ppnode =  pnode;
    return JszlE_None;
  }
  else if(*loc == '['){ 
    if(!IS_ARRAY((*pnode)) && !IS_OBJECT((*pnode)))
    {
      printf("Error: Must be structural node\n");
      *ppnode = 0;
      return JSON_ERROR_MUST_BE_ARRAY_OR_OBJECT;
    }
    loc++;
    n = is_valid_number(loc, &type, &subtype);
    idx = atouint(loc, n);
    pnode = get_value_byidx(pnode, idx);
    loc += n;
    if(!pnode) return JszlE_KeyUndef;
    if(*loc++ != ']'){
      printf("Error: Syntax\n");	
      *ppnode = 0;
      return JSON_ERROR_SYNTAX;
    }
  }
  else if(*loc == '.' || *loc == '/'){ //object
    loc++;
    if(!IS_OBJECT((*pnode))){
      printf("Error: Must be an object\n");
      *ppnode = 0;
      return JSON_ERROR_TYPE_MISMATCH;
    }
    n = get_node_byname(pnode, &pnode, loc);
    if(!n) return JszlE_KeyUndef;
    loc += n;
  }
  else { //err
  //should check if 'loc' is an object and a valid key name
    if(IS_ARRAY( (*pnode) ) || IS_OBJECT((*pnode)))
    {
      printf("Error: No namespaces\n");
      *ppnode = 0;
      return JSON_ERROR_SYNTAX;
    }
  }
  //do an atom find on the key
  //ns = json_find_ns(node);
  goto begin_loop;
}

