// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_DBO_PTR_IMPL_H_
#define WT_DBO_DBO_PTR_IMPL_H_

namespace Wt {
  namespace Dbo {
    namespace Impl {

      template <class C, typename T>
      void setAutogeneratedId(MetaDbo<C>& /* dbo */, const T& /* currentId */,
			      long long /* id */) { }

      template <class C>
      void setAutogeneratedId(MetaDbo<C>& dbo, const long long& /* currentId */,
			      long long id) {
	dbo.setId(id);
      }

      template <class C, typename T>
      long long asLongLong(const ptr<C>& /* ptr */, const T& /* id */) {
	return -1;
      }

      template <class C>
      long long asLongLong(const ptr<C>& /* ptr */, const long long& id) {
	return id;
      }

      template <class C, typename T>
      T asIdType(const ptr<C>& /* ptr */, long long /* id */, const T& invalidId)
      {
	return invalidId;
      }

      template <class C>
      long long asIdType(const ptr<C>& /* ptr */, long long id,
			 const long long& /* invalidId */)
      {
	return id;
      }
    }

template <class C>
MetaDbo<C>::~MetaDbo()
{
  if ((!isOrphaned()) && session())
    session()->prune(this);

  delete obj_;
}

template <class C>
Impl::MappingInfo *MetaDbo<C>::getMapping()
{
  return session()->template getMapping<C>();
}

template <class C>
void MetaDbo<C>::flush()
{
  checkNotOrphaned();

  if (state_ & NeedsDelete) {
    state_ &= ~NeedsDelete;

    try {
      session()->implDelete(*this);
      setTransactionState(DeletedInTransaction);
    } catch (...) {
      setTransactionState(DeletedInTransaction);
      throw;
    }
  } else if (state_ & NeedsSave) {
    state_ &= ~NeedsSave;
    state_ |= Saving;

    try {
      session()->implSave(*this);
      setTransactionState(SavedInTransaction);
    } catch (...) {
      setTransactionState(SavedInTransaction);
      throw;
    }
  } else if (state_ & Saving) {
    /*
     * This must be because of a circular relational dependency:
     *  A belongsTo(B)
     *  B belongsTo(A)
     *
     * Could also be because of A belongsTo(A) which could be perfectly
     * if the ptr is assigned after the object itself has been saved first.
     */
    // throw Exception("Wt::Dbo::ptr::flush(): circular dependency detected!");
  }
}

template <class C>
void MetaDbo<C>::bindId(SqlStatement *statement, int& column)
{
  Impl::MappingInfo *mapping = session()->template getMapping<C>();

  SaveBaseAction action(*this, *mapping, statement, column);

  field(action, id_,
	mapping->naturalIdFieldName, mapping->naturalIdFieldSize);

  column = action.column();
}

template <class C>
void MetaDbo<C>::bindModifyId(SqlStatement *statement, int& column)
{
  Impl::MappingInfo *mapping = session()->template getMapping<C>();

  SaveBaseAction action(*this, *mapping, statement, column);

  field(action, id_,
	mapping->naturalIdFieldName, mapping->naturalIdFieldSize);

  action.visitAuxIds(*obj_);

  column = action.column();
}

template <class C>
void MetaDbo<C>::bindId(std::vector<Impl::ParameterBase *>& parameters)
{
  parameters.push_back(new Impl::Parameter<typename dbo_traits<C>::IdType>(id_));
}

template <class C>
void MetaDbo<C>::prune()
{
  checkNotOrphaned();
  session()->prune(this);
  setId(dbo_traits<C>::invalidId());
  setVersion(-1);
  setState(New);
}

template <class C>
void MetaDbo<C>::doTransactionDone(bool success)
{
  Session *s = session();

  if (success) {
    if (deletedInTransaction()) {
      prune();
      setSession(nullptr);
    } else if (savedInTransaction()) {
      setVersion(version() + 1);
      setState(Persisted);
    } 
  } else {
    /*
     * This undoing of the transaction isn't consistent with objects that
     * haven't been flushed.
     */
    if (deletedInTransaction()) {
      state_ |= NeedsDelete;
      session()->needsFlush(this);
    } else if (savedInTransaction()) {
      if (isNew()) {
	prune();
      } else {
	/*
	 * If we support changing the Id, then we need to restore the old
	 * Id here.
	 */
	state_ |= NeedsSave;
	session()->needsFlush(this);
      }
    }
  }

  if (obj_)
    s->implTransactionDone(*this, success);

  resetTransactionState();
}

template <class C>
void MetaDbo<C>::purge()
{
  checkNotOrphaned();
  if (isPersisted() && !isDirty() && !inTransaction()) {
    delete obj_;
    obj_ = nullptr;
    setVersion(-1);
  }
}

template <class C>
void MetaDbo<C>::reread()
{
  checkNotOrphaned();
  if (isPersisted()) {
    session()->discardChanges(this);

    delete obj_;
    obj_ = nullptr;
    setVersion(-1);

    state_ = Persisted;
  }
}

template <class C>
void MetaDbo<C>::setObj(C *obj)
{
  checkNotOrphaned();
  obj_ = obj;
  DboHelper<C>::setMeta(*obj, this);
}

template <class C>
C *MetaDbo<C>::obj()
{
  checkNotOrphaned();
  if (!obj_ && !isDeleted())
    doLoad();

  return obj_;
}

template <class C>
int MetaDbo<C>::version() const
{
  const_cast<MetaDbo<C> *>(this)->obj(); // Load the object

  return version_;
}

template <class C>
MetaDbo<C>::MetaDbo(C *obj)
  : MetaDboBase(-1, New | NeedsSave, nullptr),
    obj_(obj),
    id_(dbo_traits<C>::invalidId())
{ 
  DboHelper<C>::setMeta(*obj_, this);
}

template <class C>
MetaDbo<C>::MetaDbo(const IdType& id, int version, int state,
		    Session& session, C *obj)
  : MetaDboBase(version, state, &session),
    obj_(obj),
    id_(id)
{
  if (obj_)
    DboHelper<C>::setMeta(*obj_, this);
}

template <class C>
MetaDbo<C>::MetaDbo(Session& session)
  : MetaDboBase(-1, Persisted, &session),
    obj_(nullptr),
    id_(dbo_traits<C>::invalidId())
{ }

template <class C>
void MetaDbo<C>::setAutogeneratedId(long long id)
{
  Impl::setAutogeneratedId(*this, id_, id);
}

template <class C>
void MetaDbo<C>::doLoad()
{
  session()->load(this);
  DboHelper<C>::setMeta(*obj_, this);
}

template <class C>
ptr<C>::mutator::mutator(MetaDbo<MutC> *obj)
  : obj_(obj)
{ 
  obj_->setDirty();
}

template <class C>
ptr<C>::mutator::~mutator()
{ 
  obj_->setDirty();
}

template <class C>
C *ptr<C>::mutator::operator->() const
{
  return obj_->obj();
}
 
template <class C>
C& ptr<C>::mutator::operator*() const
{
  return *obj_->obj();
}

template <class C>
ptr<C>::mutator::operator C*() const
{
  return obj_->obj();
}

template <class C>
ptr<C>::ptr()
  : obj_(nullptr)
{ }

template <class C>
ptr<C>::ptr(std::nullptr_t)
  : obj_(nullptr)
{ }

template <class C>
ptr<C>::ptr(std::unique_ptr<C> obj)
  : obj_(nullptr)
{
  if (obj) {
    obj_ = new MetaDbo<MutC>(const_cast<MutC*>(obj.release()));
    takeObj();
  }
}

template <class C>
ptr<C>::ptr(const ptr<C>& other)
  : obj_(other.obj_)
{
  takeObj();
}

template <class C>
template <class D, typename>
ptr<C>::ptr(const ptr<D>& other)
  : obj_(other.obj_)
{
  takeObj();
}

template <class C>
ptr<C>::ptr(ptr<C>&& other) noexcept
  : obj_(other.obj_)
{
  other.obj_ = nullptr;
}

template <class C>
template <class D, typename>
ptr<C>::ptr(ptr<D>&& other) noexcept
  : obj_(other.obj_)
{
  other.obj_ = nullptr;
}

template <class C>
ptr<C>::~ptr()
{
  freeObj();
}

template <class C>
void ptr<C>::reset(std::unique_ptr<C> obj)
{
  freeObj();
  if (obj) {
    obj_ = new MetaDbo<MutC>(const_cast<MutC*>(obj.release()));
    takeObj();
  }
}

template <class C>
ptr<C>& ptr<C>::operator= (const ptr<C>& other)
{
  if (obj_ != other.obj_) {
    freeObj();
    obj_ = other.obj_;
    takeObj();
  }

  return *this;
}

template <class C>
template <class D, typename>
ptr<C>& ptr<C>::operator= (const ptr<D>& other)
{
  if (obj_ != other.obj_) {
    freeObj();
    obj_ = other.obj_;
    takeObj();
  }

  return *this;
}

template <class C>
ptr<C>& ptr<C>::operator= (ptr<C>&& other) noexcept
{
  if (this == &other)
    return *this;

  if (obj_ == other.obj_) {
    other.freeObj();
  } else {
    freeObj();
    obj_ = other.obj_;
    other.obj_ = nullptr;
  }

  return *this;
}

template <class C>
template <class D, typename>
ptr<C>& ptr<C>::operator= (ptr<D>&& other) noexcept
{
  if (obj_ == other.obj_) {
    other.freeObj();
  } else {
    freeObj();
    obj_ = other.obj_;
    other.obj_ = nullptr;
  }

  return *this;
}

template <class C>
const C *ptr<C>::operator->() const
{
  const C *v = get();

  if (!v)
    throw Exception("Wt::Dbo::ptr<" + std::string(typeid(C).name()) + ">: null dereference");

  return v;
}

template <class C>
const C *ptr<C>::get() const
{
  if (obj_)
    return obj_->obj();
  else
    return nullptr;
}

template <class C>
const C& ptr<C>::operator*() const
{
  if (obj_)
    return *obj_->obj();
  else
    throw Exception("Wt::Dbo::ptr<" + std::string(typeid(C).name()) + ">: null dereference");
}

template <class C>
typename ptr<C>::mutator ptr<C>::modify() const
{
  if (obj_)
    return mutator(obj_);
  else
    throw Exception("Wt::Dbo::ptr<" + std::string(typeid(C).name()) + ">: null dereference");
}

template <class C>
bool ptr<C>::operator== (const ptr<MutC>& other) const
{
  return obj_ == other.obj_;
}

template <class C>
bool ptr<C>::operator== (const ptr<const C>& other) const
{
  return obj_ == other.obj_;
}

template <class C>
bool ptr<C>::operator== (const weak_ptr<MutC>& other) const
{
  return other == *this;
}

template <class C>
bool ptr<C>::operator==(const weak_ptr<const C>& other) const
{
  return other == *this;
}

template <class C>
bool ptr<C>::operator!= (const ptr<MutC>& other) const
{
  return !(*this == other);
}

template <class C>
bool ptr<C>::operator!= (const ptr<const C>& other) const
{
  return !(*this == other);
}

template <class C>
bool ptr<C>::operator!= (const weak_ptr<MutC>& other) const
{
  return !(*this == other);
}

template <class C>
bool ptr<C>::operator!= (const weak_ptr<const C>& other) const
{
  return !(*this == other);
}

template <class C>
bool ptr<C>::operator< (const ptr<MutC>& other) const
{
  return obj_ < other.obj_;
}

template <class C>
bool ptr<C>::operator< (const ptr<const C>& other) const
{
  return obj_ < other.obj_;
}

template <class C>
ptr<C>::operator bool() const
{
  return obj_ != nullptr;
}

template <class C>
void ptr<C>::flush() const
{
  if (obj_)
    obj_->flush();
}

template <class C>
void ptr<C>::purge()
{
  if (obj_)
    obj_->purge();
}

template <class C>
void ptr<C>::reread()
{
  if (obj_)
    obj_->reread();
}

template <class C>
void ptr<C>::remove()
{
  if (obj_)
    obj_->remove();
}

template <class C>
bool ptr<C>::isDirty() const
{
  if (obj_)
    return obj_->isDirty();
  else
    return false;
}

template <class C>
typename dbo_traits<C>::IdType ptr<C>::id() const
{
  if (obj_)
    return obj_->id();
  else
    return dbo_traits<C>::invalidId();
}

template <class C>
int ptr<C>::version() const
{
  if (obj_)
    return obj_->version();
  else
    return -1;
}

template <class C>
bool ptr<C>::isTransient() const
{
  if (obj_)
    return obj_->isTransient();
  else
    return true;
}

template <class C>
Session *ptr<C>::session() const
{
  if (obj_)
    return obj_->session();
  else
    return nullptr;
}

template <class C>
ptr<C>::ptr(MetaDbo<MutC> *obj)
  : obj_(obj)
{
  takeObj();
}

template <class C>
void ptr<C>::resetObj(MetaDboBase *dbo)
{
  freeObj();
  obj_ = dynamic_cast<MetaDbo<C> *>(dbo);
  takeObj();
}

template <class C>
void ptr<C>::transactionDone(bool success)
{
  obj_->transactionDone(success);
}

template <class C>
void ptr<C>::takeObj()
{
  if (obj_)
    obj_->incRef();
}

template <class C>
void ptr<C>::freeObj()
{
  if (obj_) {
    obj_->decRef();
    obj_ = nullptr;
  }
}

template <class C>
std::ostream& operator<< (std::ostream& o, const ptr<C>& ptr)
{
  if (ptr.obj_ && ptr.obj_->session())
    return o << "["
	     << ptr.obj_->session()->template tableName<C>()
	     << ": " << ptr.id()
	     << "]";
  else
    return o << "[null]";
}

template <class C>
Dbo<C>::Dbo()
  : meta_(nullptr)
{ }

template <class C>
Dbo<C>::Dbo(const Dbo<C>& /* other */)
  : meta_(nullptr)
{ }

template <class C>
typename dbo_traits<C>::IdType Dbo<C>::id() const
{
  if (meta_)
    return meta_->id();
  else
    return dbo_traits<C>::invalidId();
}

template <class C>
Session *Dbo<C>::session() const
{
  return meta_ ? meta_->session() : nullptr;
}

template <class C>
void Dbo<C>::setDirty()
{
  if (meta_)
    meta_->setDirty();
}

template <class C>
ptr<C> Dbo<C>::self() const
{
  if (meta_)
    return ptr<C>(meta_);
  else
    return ptr<C>();
}

template <class C>
bool Dbo<C>::isDirty() const
{
  if (meta_)
    return meta_->isDirty();
  else
    return false;
}

template <class C>
void query_result_traits< ptr<C> >
::getFields(Session& session, std::vector<std::string> *aliases,
	    std::vector<FieldInfo>& result)
{
  std::size_t first = result.size();
  session.getFields(session.tableName<C>(), result);

  if (aliases) {
    if (aliases->empty())
      throw Exception("Session::query(): not enough aliases for result");

    std::string alias = aliases->front();
    aliases->erase(aliases->begin());

    for (std::size_t i = first; i < result.size(); ++i)
      result[i].setQualifier(alias, i == first);
  }
}

template <class C>
ptr<C> query_result_traits< ptr<C> >
::load(Session& session, SqlStatement& statement, int& column)
{
  return session.template load<C>(&statement, column);
}

template <class C>
void query_result_traits< ptr<C> >
::getValues(const ptr<C>& ptr, std::vector<cpp17::any>& values)
{
  ToAnysAction action(values);

  action.visit(ptr);
}

template <class C>
void query_result_traits< ptr<C> >
::setValue(const ptr<C>& ptr, int& index, const cpp17::any& value)
{
  FromAnyAction action(index, value);
  action.visit(ptr);
}

template <class C>
ptr<C> query_result_traits< ptr<C> >::create()
{
  return ptr<C>(std::unique_ptr<C>(new C()));
}

template <class C>
void query_result_traits< ptr<C> >::add(Session& session, ptr<C>& ptr)
{
  session.add(ptr);
}

template <class C>
void query_result_traits< ptr<C> >::remove(ptr<C>& ptr)
{
  ptr.remove();
}

template <class C>
long long query_result_traits< ptr<C> >::id(const ptr<C>& ptr)
{
  return Impl::asLongLong(ptr, ptr.id());
}

template <class C>
ptr<C> query_result_traits< ptr<C> >::findById(Session& session, long long id)
{
  typename dbo_traits<C>::IdType C_id = dbo_traits<C>::invalidId();
  ptr<C> ptr;

  C_id = Impl::asIdType(ptr, id, C_id);

  if (!(C_id == dbo_traits<C>::invalidId()))
    ptr = session.load<C>(C_id);

  return ptr;
}

  }
}

#endif // WT_DBO_PTR_H_
