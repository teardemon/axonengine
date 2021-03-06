/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/


#ifndef AX_EDITOR_HISTORY_H
#define AX_EDITOR_HISTORY_H

namespace Axon { namespace Editor {

	typedef List<Action*>		ActionList;
	typedef shared_ptr<Action>	ActionPtr;
	typedef List<ActionPtr>		HistoryList;

	class AX_API History : public Action {
	public:
		History(Context* context) : Action(context) {}
		History(Context* context, const String& msg) : Action(context), m_message(msg) {}
		virtual ~History() {}

		virtual void doIt() = 0;
		virtual void undo() { /* do nothing default */}
		virtual bool isUndoable() { return true; }
		virtual String getName() { return "History"; }
		virtual String getMessage() { return m_message; }
		virtual void setMessage(const String& msg) { m_message = msg; }
		virtual int getMemoryUsed() { return 0; }


	protected:
		String m_message;
	};

	//--------------------------------------------------------------------------
	// class HistoryManager
	//--------------------------------------------------------------------------

	class AX_API HistoryManager {
	public:
		HistoryManager();
		~HistoryManager();

		void addHistory(Action* his);
		void undo(int count);
		void redo(int count);

		void clear();

		HistoryList getUndoList() { return m_undoList; }
		HistoryList getRedoList() { return m_redoList; }

	private:
		HistoryList m_undoList;
		HistoryList m_redoList;
	};

	//--------------------------------------------------------------------------
	// class TransformHis
	//--------------------------------------------------------------------------

	class TransformHis : public History {
	public:
		TransformHis(Context* context, const String& msg, int actorId, const AffineMat& oldmat, const AffineMat& newmat);
		virtual ~TransformHis();

		virtual void doIt();
		virtual void undo();
		virtual bool isUndoable();
		virtual String getName();
		virtual int getMemoryUsed();

	private:
		int m_actorId;
		AffineMat m_oldMat;
		AffineMat m_newMat;
	};

	//--------------------------------------------------------------------------
	// class GroupHis
	//--------------------------------------------------------------------------

	class GroupHis : public History {
	public:
		GroupHis(Context* context, const String& msg);
		virtual ~GroupHis();

		virtual void doIt();
		virtual void undo();
		virtual bool isUndoable();
		virtual String getName();
		virtual int getMemoryUsed();

		void append(Action* his);

	private:
		ActionList m_actionList;
	};


	//--------------------------------------------------------------------------
	// class DeleteHis
	//--------------------------------------------------------------------------

	class DeleteHis : public History {
	public:
		DeleteHis(Context* context, const String& msg, const ActorList& actorlist);
		virtual ~DeleteHis() {}

		virtual void doIt();
		virtual void undo();
		virtual String getName() { return "DeleteHis"; }
		virtual int getMemoryUsed();

	private:
		ActorList m_actorList;
	};

	//--------------------------------------------------------------------------
	// class UndeleteHis
	//--------------------------------------------------------------------------

	class UndeleteHis : public History {
	public:
		UndeleteHis(Context* context, const String& msg, const ActorList& actorlist);
		virtual ~UndeleteHis() {}

		virtual void doIt();
		virtual void undo();
		virtual String getName() { return "UndeleteHis"; }
		virtual int getMemoryUsed();

	private:
		ActorList m_actorList;
	};

	//--------------------------------------------------------------------------
	// class PropertyEditHis
	//--------------------------------------------------------------------------

	class PropertyEditHis : public History {
	public:
		PropertyEditHis(Context* context, Actor* actor, const String& propname, const Variant& oldvalue, const Variant& newvalue);
		virtual ~PropertyEditHis();

		virtual void doIt();
		virtual void undo();
		virtual String getName() { return "PropertyEditHis"; }
		virtual bool isUndoable() { return true; }
		virtual int getMemoryUsed();

	private:
		Actor* m_actor;
		const String m_propName;
		const Variant m_oldValue;
		const Variant m_newValue;
	};

	//--------------------------------------------------------------------------
	// class SelectHis
	//--------------------------------------------------------------------------

	class SelectHis : public History {
	public:
		SelectHis(Context* context, const ActorList& oldlist, const ActorList& newlist);
		virtual ~SelectHis();

		virtual void doIt();
		virtual void undo();
		virtual String getName() { return "SelectHis"; }
		virtual bool isUndoable() { return true; }
		virtual int getMemoryUsed();

	private:
		ActorList m_oldlist;
		ActorList m_newlist;
	};

}} // namespace Axon::Editor

#endif

