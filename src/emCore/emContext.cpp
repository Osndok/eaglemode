//------------------------------------------------------------------------------
// emContext.cpp
//
// Copyright (C) 2005-2008 Oliver Hamann.
//
// Homepage: http://eaglemode.sourceforge.net/
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 3 as published by the
// Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License version 3 for
// more details.
//
// You should have received a copy of the GNU General Public License version 3
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------------

#include <emCore/emModel.h>


emContext::emContext(emContext & parentContext)
	: emEngine(parentContext.GetScheduler()),
	RootContext(parentContext.RootContext)
{
	ParentContext=&parentContext;
	FirstChildContext=NULL;
	LastChildContext=NULL;
	PrevContext=ParentContext->LastChildContext;
	NextContext=NULL;
	if (PrevContext) PrevContext->NextContext=this;
	else ParentContext->FirstChildContext=this;
	ParentContext->LastChildContext=this;
	AvlTree=NULL;
	ModelCount=0;
	DoGCOnModels=false;
	SharedTiming=parentContext.SharedTiming;
}


emContext::~emContext()
{
	emModel * m;
	int minHash;

	CrossPtrList.BreakCrossPtrs();

	while (LastChildContext) delete LastChildContext;

	while (AvlTree) {
		m=SearchUnused();
		if (!m) {
			emFatalError(
				"emContext: Could not free all common models at destruction. Probably\n"
				"there are circular or outer references. Remaining common models are:\n%s",
				GetListing().Get()
			);
		}
		do {
			minHash=m->AvlHashCode;
			UnregisterModel(m);
			if (!AvlTree) break;
			m=SearchUnused(minHash);
		} while(m);
	}

	if (ModelCount) {
		emFatalError(
			"emContext: Remaining private models after destruction.\n"
		);
	}

	if (LastChildContext) {
		emFatalError(
			"emContext: Remaining child contexts after destruction (constructed by model destructor?).\n"
		);
	}

	if (ParentContext) {
		if (PrevContext) PrevContext->NextContext=NextContext;
		else ParentContext->FirstChildContext=NextContext;
		if (NextContext) NextContext->PrevContext=PrevContext;
		else ParentContext->LastChildContext=PrevContext;
		PrevContext=NULL;
		NextContext=NULL;
		ParentContext=NULL;
	}
	else {
		delete SharedTiming;
		SharedTiming=NULL;
	}
}


emModel * emContext::Lookup(const type_info & modelClass, const char * name)
{
	EM_AVL_SEARCH_VARS(emModel)
	int d, hashCode;

	hashCode=CalcHashCode(modelClass,name);

	EM_AVL_SEARCH_BEGIN(emModel,AvlNode,AvlTree)
		d=hashCode-element->AvlHashCode;
		if (!d) {
			d=strcmp(name,element->Name.Get());
			if (!d) {
				d=strcmp(
					emRawNameOfTypeInfo(modelClass),
					emRawNameOfTypeInfo(typeid(*element))
				);
				if (!d) return element;
			}
		}
		if (d<0) EM_AVL_SEARCH_GO_LEFT
		else EM_AVL_SEARCH_GO_RIGHT
	EM_AVL_SEARCH_END
	return NULL;
}


emModel * emContext::LookupInherited(
	const type_info & modelClass, const char * name
)
{
	emContext * c;
	emModel * m;

	c=this;
	do {
		m=c->Lookup(modelClass,name);
		if (m) break;
		c=c->ParentContext;
	} while (c);
	return m;
}


emString emContext::GetListing() const
{
	EM_AVL_LOOP_VARS(emModel)
	emString listing;

	EM_AVL_LOOP_START(emModel,AvlNode,AvlTree)
		listing+=emString::Format(
			" class=%s name=\"%s\"\n",
			typeid(*element).name(),
			element->Name.Get()
		);
	EM_AVL_LOOP_END
	return listing;
}


void emContext::GetModelInfo(
	int * pCommonCount, int * pPrivateCount, emModel * * * pArrayOfCommon
)
{
	EM_AVL_LOOP_VARS(emModel)
	emModel * * array;
	int i,commonCount;

	commonCount=0;
	EM_AVL_LOOP_START(emModel,AvlNode,AvlTree)
		commonCount++;
	EM_AVL_LOOP_END

	if (pCommonCount) *pCommonCount=commonCount;
	if (pPrivateCount) *pPrivateCount=ModelCount-commonCount;

	if (pArrayOfCommon) {
		array=new emModel*[commonCount];
		i=0;
		EM_AVL_LOOP_START(emModel,AvlNode,AvlTree)
			array[i++]=element;
		EM_AVL_LOOP_END
		*pArrayOfCommon=array;
	}
}


bool emContext::Cycle()
{
	return false;
}


void emContext::RegisterModel(emModel * model)
{
	EM_AVL_INSERT_VARS(emModel)
	int d, hashCode;

	if (model->AvlHashCode) return;

	hashCode=CalcHashCode(typeid(*model),model->Name);

	EM_AVL_INSERT_BEGIN_SEARCH(emModel,AvlNode,AvlTree)
		d=hashCode-element->AvlHashCode;
		if (!d) {
			d=strcmp(model->Name.Get(),element->Name.Get());
			if (!d) {
				d=strcmp(
					emRawNameOfTypeInfo(typeid(*model)),
					emRawNameOfTypeInfo(typeid(*element))
				);
				if (!d) {
					emFatalError(
						"emContext: Two common models with same identity: class=%s name=\"%s\"",
						typeid(*model).name(),
						model->Name.Get()
					);
				}
			}
		}
		if (d<0) EM_AVL_INSERT_GO_LEFT
		else EM_AVL_INSERT_GO_RIGHT
	EM_AVL_INSERT_END_SEARCH
		model->AvlHashCode=hashCode;
		model->Alloc();
		element=model;
	EM_AVL_INSERT_NOW(AvlNode)

	if (model->RefCount==1 && ((int)model->MinCommonLifetime)>=0) {
		model->TimeOfDeath=SharedTiming->SecsCounter+model->MinCommonLifetime;
		DoGCOnModels=true;
	}
}


void emContext::UnregisterModel(emModel * model)
{
	EM_AVL_REMOVE_VARS(emModel)
	int d, hashCode;

	if (!model->AvlHashCode) return;

	hashCode=model->AvlHashCode;

	EM_AVL_REMOVE_BEGIN(emModel,AvlNode,AvlTree)
		d=hashCode-element->AvlHashCode;
		if (!d) {
			if (element==model) {
				EM_AVL_REMOVE_NOW
				model->AvlHashCode=0;
				model->Free();
				break;
			}
			d=strcmp(model->Name.Get(),element->Name.Get());
			if (!d) {
				d=strcmp(
					emRawNameOfTypeInfo(typeid(*model)),
					emRawNameOfTypeInfo(typeid(*element))
				);
			}
		}
		if (d<0) EM_AVL_REMOVE_GO_LEFT
		else EM_AVL_REMOVE_GO_RIGHT
	EM_AVL_REMOVE_END
}


emContext::emContext(emScheduler & scheduler)
	: emEngine(scheduler),
	RootContext(*(emRootContext*)this)
{
	ParentContext=NULL;
	FirstChildContext=NULL;
	LastChildContext=NULL;
	PrevContext=NULL;
	NextContext=NULL;
	AvlTree=NULL;
	ModelCount=0;
	DoGCOnModels=false;
	SharedTiming=new SharedTimingEngine(
		*(emRootContext*)this,
		10 //??? Maybe this should be configurable (seconds between garbage collections)
	);
}


int emContext::CalcHashCode(const type_info & modelClass, const char * name)
{
	int h;

	// Remember, we are doing things like d=hash1-hash2. Therefore the "&=INT_MAX".
	// Besides, the hash code must not be zero.
	h=emCalcHashCode(emRawNameOfTypeInfo(modelClass));
	h=emCalcHashCode(name,h);
	h&=INT_MAX;
	if (!h) h=1;
	return h;
}


emModel * emContext::SearchUnused()
{
	EM_AVL_LOOP_VARS(emModel)

	EM_AVL_LOOP_START(emModel,AvlNode,AvlTree)
		if (element->RefCount<=1) return element;
	EM_AVL_LOOP_END
	return NULL;
}


emModel * emContext::SearchUnused(int minHash)
{
	emAvlIterator iter;
	EM_AVL_ITER_VARS(emModel)

	EM_AVL_ITER_START_ANY_BEGIN(emModel,AvlNode,AvlTree,iter)
		if (minHash<=element->AvlHashCode) {
			EM_AVL_ITER_START_ANY_GO_LEFT_OR_FOUND(iter)
		}
		else {
			EM_AVL_ITER_START_ANY_GO_RIGHT_OR_FOUND(iter)
		}
	EM_AVL_ITER_START_ANY_END
	while (element) {
		if (element->RefCount<=1) return element;
		EM_AVL_ITER_NEXT(emModel,AvlNode,iter)
	}
	return NULL;
}


emModel * emContext::SearchGarbage()
{
	EM_AVL_LOOP_VARS(emModel)
	unsigned int t;

	t=SharedTiming->SecsCounter;
	EM_AVL_LOOP_START(emModel,AvlNode,AvlTree)
		if (
			element->RefCount<=1 &&
			((int)element->MinCommonLifetime)>=0
		) {
			if (((int)(element->TimeOfDeath-t))<0) {
				return element;
			}
			DoGCOnModels=true;
		}
	EM_AVL_LOOP_END
	return NULL;
}


emModel * emContext::SearchGarbage(int minHash)
{
	emAvlIterator iter;
	EM_AVL_ITER_VARS(emModel)
	unsigned int t;

	EM_AVL_ITER_START_ANY_BEGIN(emModel,AvlNode,AvlTree,iter)
		if (minHash<=element->AvlHashCode) {
			EM_AVL_ITER_START_ANY_GO_LEFT_OR_FOUND(iter)
		}
		else {
			EM_AVL_ITER_START_ANY_GO_RIGHT_OR_FOUND(iter)
		}
	EM_AVL_ITER_START_ANY_END

	t=SharedTiming->SecsCounter;
	while (element) {
		if (
			element->RefCount<=1 &&
			((int)element->MinCommonLifetime)>=0
		) {
			if (((int)(element->TimeOfDeath-t))<0) {
				return element;
			}
			DoGCOnModels=true;
		}
		EM_AVL_ITER_NEXT(emModel,AvlNode,iter)
	}
	return NULL;
}


void emContext::CollectGarbage()
{
	emContext * c;
	emModel * m;
	int minHash;

	if (DoGCOnModels) {
		emDLog("emContext %p: Garbage Collection...",this);
		DoGCOnModels=false;
		while (AvlTree) {
			m=SearchGarbage();
			if (!m) break;
			do {
				if (emIsDLogEnabled()) {
					emDLog(
						"emContext: Removing by GC: class=\"%s\" name=\"%s\"",
						typeid(*m).name(),
						m->Name.Get()
					);
				}
				minHash=m->AvlHashCode;
				UnregisterModel(m);
				if (!AvlTree) break;
				m=SearchGarbage(minHash);
			} while(m);
		}
	}
	for (c=FirstChildContext; c; c=c->NextContext) {
		c->CollectGarbage();
	}
}


emContext::SharedTimingEngine::SharedTimingEngine(
	emRootContext & rootContext, unsigned int gcPeriod
)
	: emEngine(rootContext.GetScheduler()),
	RootContext(rootContext),
	SecsTimer(rootContext.GetScheduler())
{
	GCPeriod=gcPeriod;
	SecsCounter=0;
	TimeOfGC=GCPeriod;
	SecsTimer.Start(1000,true);
	AddWakeUpSignal(SecsTimer.GetSignal());
}


emContext::SharedTimingEngine::~SharedTimingEngine()
{
}


bool emContext::SharedTimingEngine::Cycle()
{
	if (IsSignaled(SecsTimer.GetSignal())) {
		SecsCounter++;
		if (((int)(TimeOfGC-SecsCounter))<=0) {
			RootContext.CollectGarbage();
			TimeOfGC=SecsCounter+GCPeriod;
		}
	}
	return false;
}
