/*=======================================================================
   Copyright (c) 2018 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <stdlib.h>
# include <sig/gs_graph.h>
# include <sig/gs_string.h>
# include <sig/gs_heap.h>

//# define GS_USE_TRACE1 // Node operations
//# define GS_USE_TRACE2 // Search
# include <sig/gs_trace.h>

//============================== GsGraphNode =====================================

GsGraphNode::GsGraphNode ()
{
	_index=0;
	_graph=0;
	_blocked=0;
}

GsGraphNode::~GsGraphNode ()
{
	if ( _graph )
	{	GsManagerBase* lman = _graph->link_class_manager();
		while ( _links.size()>0 ) lman->free ( _links.pop() );
	}
}

GsGraphLink* GsGraphNode::linkto ( GsGraphNode* n, float cost )
{
	GsGraphLink* l = (GsGraphLink*) _graph->link_class_manager()->alloc();
	l->_node = n;
	l->_index = 0;
	l->_cost = cost;
	_links.push () = l;
	return l;
}

void GsGraphNode::unlink ()
{
	GS_TRACE1 ( "GsGraphNode::unlink..." );

	GsManagerBase* lman = _graph->link_class_manager();
	while ( _links.size() )
	{	GsGraphNode* n = _links.top()->node();
		// remove link this => n:
		lman->free ( _links.pop() );
		// remove link n => this
		int lnt = n->search_link(this);
		if ( lnt>=0 )
		{	lman->free ( n->_links[lnt] );
			n->_links[lnt] = n->_links.pop(); // fast remove
		}
	}
	GS_TRACE1 ( "Done." );
}

void GsGraphNode::unlink ( int li )
{
	GsManagerBase* lman = _graph->link_class_manager();
	lman->free ( _links[li] );
	_links[li] = _links.pop();
}

int GsGraphNode::search_link ( GsGraphNode* n ) const
{
	int i;
	for ( i=_links.size()-1; i>=0; i-- )
		if ( _links[i]->node()==n ) break;
	return i;
}

void GsGraphNode::output ( GsOutput& o ) const
{
	float cost;
	int i, max;
	max = _links.size()-1;
	if ( max<0 ) return;

	GsManagerBase* lman = _graph->link_class_manager();

	o<<'(';
	for ( i=0; i<=max; i++ )
	{	// output blocked status
		o << _links[i]->_blocked << gspc;

		// output index to target node
		o << _links[i]->_node->_index << gspc;

		// output link cost
		cost = _links[i]->_cost;
		if ( cost==GS_TRUNC(cost) ) o << int(cost);
		else o << cost;

		// output user data
		o << gspc;
		lman->output(o,_links[i]);

		if ( i<max ) o<<gspc;
	}
	o<<')';
}

//============================== GsGraphPathTree ===============================================

class GsGraphPathTree
{  public :
//	struct Node { int parent; float cost; GsGraphNode* node; };
	struct Node { int parent; GsGraphNode* node; };
	struct Leaf { int l; int d; }; // the leaf index in nodes array, and its depth
	GsArray<Node> N;
	GsHeap<Leaf,float> Q;
	GsGraphBase* graph;
	GsGraphNode* closest;
	int iclosest;
	float cdist;
	float (*distfunc) ( const GsGraphNode*, const GsGraphNode*, void* udata );
	void *udata;
	bool bidirectional_block;

   public :
	GsGraphPathTree ()
	{	bidirectional_block = false;
	}

	void init ( GsGraphBase* g, GsGraphNode* n )
	{	N.size(1);
		N[0].parent = -1;
		N[0].node = n;
		n->fparam = 0; // cost to come
		g->mark ( n );
		Leaf l;
		l.l = l.d = 0;
		Q.init ();
		Q.insert ( l, 0 );
		graph = g;
		distfunc = 0;
		udata = 0;
		closest = 0;
		iclosest = 0;
		cdist = 0;
	}

	bool has_leaf () const { return Q.size()>=0; }

	float& ncost ( int i ) { return N[i].node->fparam; }

	bool expand_lowest_cost_leaf ( GsGraphNode* goalnode )
	{	int n = Q.top().l;
		int nextd = Q.top().d+1;
		Q.remove ();
		GsGraphNode* node = N[n].node;
		if ( node==goalnode ) { N.size(n+1); return true; }
		Leaf leaf;
		const GsArray<GsGraphLink*>& a = node->links();
		for ( int i=0,s=a.size(); i<s; i++ )
		{	//gsout<<a.size()<<gsnl;
			GsGraphLink* li = a[i];
			GsGraphNode* lin = li->node();
			if ( li->blocked() || lin->blocked() ) continue;
			if ( bidirectional_block && lin->link(node)->blocked() ) continue;
			float newcost = node->fparam + li->cost();
			if ( graph->marked(lin) && newcost>=lin->fparam ) continue;
			lin->fparam = newcost;
			graph->mark ( lin );
			N.push();
			N.top().parent = n;
			N.top().node = lin;
			leaf.l = N.size()-1;
			leaf.d = nextd;
			Q.insert ( leaf, lin->fparam );
			if ( distfunc )
			{	float d = distfunc ( lin, goalnode, udata );
				if ( !closest || d<cdist )
				{	closest=N.top().node; iclosest=N.size()-1; cdist=d; }
			}
		}
		return false;
	 }

	float make_path ( int i, GsArray<GsGraphNode*>& path )
	{	float cost = N[i].node->fparam;   
		path.size(0);
		while ( i>=0 )
		{	path.push() = N[i].node;
			i = N[i].parent;
		}
		return cost;
	}
};

//============================== GsGraphBase ===============================================

# define MARKFREE 0
# define MARKING  1
# define INDEXING 2

GsGraphBase::GsGraphBase ( GsManagerBase* nm, GsManagerBase* lm )
			:_nodes(nm)
{
	_curmark = 1;
	_mark_status = MARKFREE;
	_pt = 0; // allocated only if shortest path is called
	_lman = lm;
	_lman->ref(); // nm is managed by the list _nodes
	_leave_indices_after_save = 0;
}

GsGraphBase::~GsGraphBase ()
{
	_nodes.init (); // Important: this ensures that _lman is used before _lman->unref()
	delete _pt;
	_lman->unref();
}

void GsGraphBase::init ()
{
	_nodes.init();
	_curmark = 1;
	_mark_status = MARKFREE;
}

void GsGraphBase::compress ()
{
	if ( _nodes.empty() ) return;
	_nodes.gofirst();
	do { _nodes.cur()->compress();
	   } while ( _nodes.notlast() );
}

int GsGraphBase::num_links () const
{
	if ( _nodes.empty() ) return 0;

	int n=0;

	GsGraphNode* first = _nodes.first();
	GsGraphNode* cur = first;
	do { n += cur->num_links();
	 	 cur = cur->next();
	   } while ( cur!=first );

	return n;
}

//----------------------------------- marking --------------------------------

static void _errormsg ( const char* s1, int code )
{
	const char* s2= code==0? "is locked" :
					code==1? "marking is not active" :
					code==2? "indexing is not active" : "error";
	gsout.fatal ( "GsGraphBase::%s: %s!", s1, s2 );
}

void GsGraphBase::begin_marking () const
{
	if ( _mark_status!=MARKFREE ) _errormsg("begin_mark()",0);
	_mark_status = MARKING;
	if ( _curmark==gsuintmax ) _normalize_mark ();
	else _curmark++;
}

void GsGraphBase::end_marking () const
{
	_mark_status=MARKFREE;
}

bool GsGraphBase::marked ( GsGraphNode* n ) const
{
	if ( _mark_status!=MARKING ) _errormsg("marked(n)",1);
	return n->_index==_curmark;
}

void GsGraphBase::mark ( GsGraphNode* n ) const
{ 
	if ( _mark_status!=MARKING ) _errormsg("mark(n)",1);
	n->_index = _curmark;
}

void GsGraphBase::unmark ( GsGraphNode* n ) const
{
	if ( _mark_status!=MARKING ) _errormsg("unmark(n)",1);
	n->_index = _curmark-1;
}

bool GsGraphBase::marked ( GsGraphLink* l ) const
{
	if ( _mark_status!=MARKING ) _errormsg("marked(l)",1);
	return l->_index==_curmark? true:false;
}

void GsGraphBase::mark ( GsGraphLink* l ) const
{ 
	if ( _mark_status!=MARKING ) _errormsg("mark(l)",1);
	l->_index = _curmark;
}

void GsGraphBase::unmark ( GsGraphLink* l ) const
{
	if ( _mark_status!=MARKING ) _errormsg("unmark(l)",1);
	l->_index = _curmark-1;
}

//----------------------------------- indexing --------------------------------

void GsGraphBase::begin_indexing () const
{
	if ( _mark_status!=MARKFREE ) _errormsg("begin_indexing()",0);
	_mark_status = INDEXING;
}

void GsGraphBase::end_indexing () const
{
	_normalize_mark();
	_mark_status=MARKFREE;
}

gsuint GsGraphBase::index ( GsGraphNode* n ) const
{
	if ( _mark_status!=INDEXING ) _errormsg("index(n)",2);
	return n->_index;
}

void GsGraphBase::index ( GsGraphNode* n, gsuint i ) const
{
	if ( _mark_status!=INDEXING ) _errormsg("index(n,i)",2);
	n->_index = i;
}

gsuint GsGraphBase::index ( GsGraphLink* l ) const
{
	if ( _mark_status!=INDEXING ) _errormsg("index(l)",2);
	return l->_index;
}

void GsGraphBase::index ( GsGraphLink* l, gsuint i ) const
{
	if ( _mark_status!=INDEXING ) _errormsg("index(l,i)",2);
	l->_index = i;
}

//----------------------------------- construction --------------------------------

GsGraphNode* GsGraphBase::insert ( GsGraphNode* n )
{
	_nodes.insert_next ( n );
	n->_graph = this;
	return n;
}

GsGraphNode* GsGraphBase::extract ( GsGraphNode* n )
{
	_nodes.cur(n);
	return _nodes.extract();
}

void GsGraphBase::remove_node ( GsGraphNode* n )
{
	_nodes.cur(n);
	_nodes.remove();
}

int GsGraphBase::remove_link ( GsGraphNode* n1, GsGraphNode* n2 )
{
	int i, n=0;

	while ( true )
	{	i = n1->search_link(n2);
		if ( i<0 ) break;
		n++;
		n1->unlink(i);
	}

	while ( true )
	{	i = n2->search_link(n1);
		if ( i<0 ) break;
		n++;
		n2->unlink(i);
	}

	return n;
}

void GsGraphBase::link ( GsGraphNode* n1, GsGraphNode* n2, float c )
{
	n1->linkto(n2,c);
	n2->linkto(n1,c);
}

//----------------------------------- get edges ----------------------------------

void GsGraphBase::get_directed_edges ( GsArray<GsGraphNode*>& edges )
{
	edges.size ( num_nodes() );
	edges.size ( 0 );

	int i;
	GsGraphNode* n;
	GsListIterator<GsGraphNode> it(_nodes);

	for ( it.first(); it.inrange(); it.next() )
	{	n = it.get();
		for ( i=0; i<n->num_links(); i++ )
		{	edges.push() = n;
			edges.push() = n->link(i)->node();
		}
	}
}

void GsGraphBase::get_undirected_edges ( GsArray<GsGraphNode*>& edges )
{
	edges.size ( num_nodes() );
	edges.size ( 0 );

	int i, li;
	GsGraphNode* n;
	GsListIterator<GsGraphNode> it(_nodes);

	begin_marking();

	for ( it.first(); it.inrange(); it.next() )
	{	n = it.get();
		for ( i=0; i<n->links().size(); i++ )
		{	if ( !marked(n->link(i)) )
			{	edges.push() = n;
				edges.push() = n->link(i)->node();
				mark ( n->link(i) );
				li = edges.top()->search_link(n);
				if ( li>=0 ) mark ( edges.top()->link(li) );
			}
		}
	}

	end_marking();
}

//----------------------------------- components ----------------------------------

static void _traverse ( GsGraphBase* graph,
						GsArray<GsGraphNode*>& stack,
						GsArray<GsGraphNode*>& nodes )
{
	int i;
	GsGraphNode *n, *ln;
	while ( stack.size()>0 )
	{	n = stack.pop();
		graph->mark ( n );
		nodes.push() = n;
		for ( i=0; i<n->num_links(); i++ )
		{	ln = n->link(i)->node();
			if ( !graph->marked(ln) ) stack.push()=ln;
		}
	}
}

void GsGraphBase::get_connected_nodes ( GsGraphNode* source, GsArray<GsGraphNode*>& nodes )
{
	nodes.size ( num_nodes() );
	nodes.size ( 0 );

	GsArray<GsGraphNode*>& stack = _buffer;
	stack.size ( 0 );
   
	begin_marking();
	stack.push() = source;
	_traverse ( this, stack, nodes );
	end_marking();
}

void GsGraphBase::get_disconnected_components ( GsArray<int>& components, GsArray<GsGraphNode*>& nodes )
{
	nodes.size ( num_nodes() );
	nodes.size ( 0 );

	components.size ( 0 );

	GsArray<GsGraphNode*>& stack = _buffer;
	stack.size ( 0 );
   
	GsGraphNode* n;
	GsListIterator<GsGraphNode> it(_nodes);

	begin_marking();

	for ( it.first(); it.inrange(); it.next() )
	{	n = it.get();
		if ( !marked(n) )
		{	components.push() = nodes.size();
			stack.push() = n;
			_traverse ( this, stack, nodes );
			components.push() = nodes.size()-1;
		}
	}

	end_marking();
}

//----------------------------------- shortest path ----------------------------------

bool GsGraphBase::shortest_path
				 ( GsGraphNode* n1, GsGraphNode* n2, GsArray<GsGraphNode*>& path, float& cost,
				   float (*distfunc) ( const GsGraphNode*, const GsGraphNode*, void* udata ),
				   void* udata )
{
	GS_TRACE2 ( "search_shortest_path starting..." );
	path.size(0);

	if ( n1==n2 )
	{	GS_TRACE2 ( "n1==n2." );
		path.push()=n1;
		return 0;
	}

	if ( !_pt ) _pt = new GsGraphPathTree;

	GS_TRACE2 ( "initializing..." );
	begin_marking ();
	_pt->init ( this, n2 );
	if ( distfunc ) { _pt->distfunc=distfunc; _pt->udata=udata; }
	GS_TRACE2 ( "searching..." );
	while ( _pt->has_leaf() )
	{	if ( _pt->expand_lowest_cost_leaf(n1) ) break;
	}
	end_marking ();

	if ( _pt->N.top().node==n1 ) // found
	{	cost = _pt->make_path ( _pt->N.size()-1, path );
		GS_TRACE2 ( "Found! size:"<<path.size()<<" cost:"<<cost );
		return true;
	}
	else if ( distfunc) // not found but closest goal available
	{	GS_TRACE2 ( "Closest returned." );
		cost = _pt->make_path ( _pt->iclosest, path );
		return false;
	}
	else // not found
	{	GS_TRACE2 ( "Not Found." );
		cost = 0;
		return false;
	}
}

bool GsGraphBase::local_search ( GsGraphNode* startn, GsGraphNode* endn,
								int maxdepth, float maxdist, int& depth, float& dist )
{
	depth=0;
	dist=0;

	if ( startn==endn ) return true;

	begin_marking ();

	if ( !_pt ) _pt = new GsGraphPathTree;
	_pt->init ( this, startn );

	bool not_found = false;
	bool end = false;

	while ( !end )
	{	if ( !_pt->has_leaf() ) { not_found=true; break; } // not found!

		dist = _pt->ncost ( _pt->Q.top().l );
		depth = _pt->Q.top().d;

		if ( maxdepth>0 && depth>maxdepth ) { break; } // max depth reached
		if ( maxdist>0 && dist>maxdist ) { break; }	// max dist reached

		end = _pt->expand_lowest_cost_leaf ( endn );
	}

	end_marking ();

	if ( not_found ) return false; // not found!
	return true;
}

void GsGraphBase::bidirectional_block_test ( bool b )
{
	if ( !_pt ) _pt = new GsGraphPathTree;
	_pt->bidirectional_block = b;
}

//------------------------------------- I/O --------------------------------

void GsGraphBase::output ( GsOutput& o ) const
{
	GsListIterator<GsGraphNode> it(_nodes);
	GsManagerBase* nman = node_class_manager();

	// set indices
	if ( _mark_status!=MARKFREE ) gsout.fatal("GsGraphBase::operator<<(): begin_indexing() is locked!");
	gsuint i=0;
	begin_indexing();
	for ( it.first(); it.inrange(); it.next() ) index(it.get(),i++);

	// print
	o<<'[';
	for ( it.first(); it.inrange(); it.next() )
	{	o << it->index() << gspc;			 // output node index
		o << (it->_blocked?'b':'f') << gspc;  // output node blocked status
		nman->output ( o, it.get() );		 // output user data
		if ( it.get()->num_links()>0 )		// output node links (blocked, ids, cost, and udata)
		{	o<<gspc;
			it->GsGraphNode::output ( o );
		}
		if ( !it.inlast() ) o << gsnl;
	}
	o<<']';

	if ( !_leave_indices_after_save ) end_indexing();
	_leave_indices_after_save = 0;
}

static void set_blocked ( int& blocked, const char* ltoken )
{
	// accepts letters or 0/1 integer
	if ( ltoken[0]=='b' || ltoken[0]=='B' ) // blocked
		blocked = 1;
	else if ( ltoken[0]=='f' || ltoken[0]=='F' ) // free
		blocked = 0;
	else // should be an integer 0/1
		blocked = atoi(ltoken);
}

void GsGraphBase::input ( GsInput& inp )
{
	GsArray<GsGraphNode*>& nodes = _buffer;
	nodes.size(128);
	nodes.size(0);

	GsManagerBase* nman = node_class_manager();
	GsManagerBase* lman = link_class_manager();

	init ();

	inp.get(); // [
	inp.get(); // get node counter (which is not needed)

	while ( inp.ltoken()[0]!=']' )
	{ 
		nodes.push() = _nodes.insert_next(); // allocate one node
		nodes.top()->_graph = this;

		inp.get(); // get node blocked status
		set_blocked ( nodes.top()->_blocked, inp.ltoken() );

		nman->input ( inp, nodes.top() ); // read node user data
	  
		inp.get(); // new node counter, or '(', or ']'

		if ( inp.ltoken()[0]=='(' )
		while ( true )
		{	inp.get();
			if ( inp.ltoken()[0]==')' ) { inp.get(); break; }
			GsArray<GsGraphLink*>& la = nodes.top()->_links;
			la.push() = (GsGraphLink*) lman->alloc();
			set_blocked ( la.top()->_blocked, inp.ltoken() ); // get link blocked status

			inp.get(); // get id
			la.top()->_index = inp.ltoken().atoi(); // store id
			inp.get(); // get cost
			la.top()->_cost = inp.ltoken().atof(); // store cost
			lman->input ( inp, la.top() );
		} 
	}

	// now convert indices to pointers:
	int i, j;
	for ( i=0; i<nodes.size(); i++ )
	{	GsArray<GsGraphLink*>& la = nodes[i]->_links;
		for ( j=0; j<la.size(); j++ )
		{ la[j]->_node = nodes[ int(la[j]->_index) ];
		}
	}
}

//---------------------------- private methods --------------------------------

// set all indices (nodes and links) to 0 and curmark to 1
void GsGraphBase::_normalize_mark() const
{
	int i;
	GsGraphNode* n;

	if ( _nodes.empty() ) return;

	_nodes.gofirst();
	do
	{	n = _nodes.cur();
		n->_index = 0;
		for ( i=0; i<n->num_links(); i++ ) n->link(i)->_index=0;
	} while ( _nodes.notlast() );
	_curmark = 1;
}

//============================== end of file ===============================

