/*
 * Copyright (C) 2010-2011 Dmitry Marakasov
 *
 * This file is part of glosm.
 *
 * glosm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * glosm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with glosm.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef ID_MAP_HH
#define ID_MAP_HH

#include <vector>
#include <cassert>

/**
 * Custom std::map-like container for storing OSM data effeciently.
 *
 * Uses lower bits of object id as a hash for no calculation overhead
 * and pooled data storage to pack elements effeciently.
 *
 * Interface and usage semantics are the same as for std::map
 */
template <typename I, typename T, int PAGE_SIZE = 1048576>
class id_map {
public:
	typedef I                                key_type;
	typedef T                                mapped_type;
	typedef std::pair<const I, T>            value_type;

	typedef value_type*                      pointer;
	typedef const value_type*                const_pointer;
	typedef value_type&                      reference;
	typedef const value_type&                const_reference;

private:
	struct hash_node {
		value_type data;
		hash_node* next;
	};

	typedef hash_node*                       hash_node_ptr;
	typedef const hash_node*                 const_hash_node_ptr;

	class page {
	public:
		typedef hash_node* iterator;
		typedef const hash_node* const_iterator;

	public:
		page() : count_(0), data_(reinterpret_cast<hash_node*>(::operator new(PAGE_SIZE))) {
			assert(sizeof(hash_node) <= PAGE_SIZE);
		}

		/* copy ctor and operator = transfer data ownership
		 * this is rather ugly, but I it seems like the best way
		 * to do it conveniently with stl containers (else, we'll
		 * have to use vector of pointers and spend extra code on
		 * exception safety and stuff */
		page(const page& other) : count_(other.count_), data_(other.data_) {
			other.count_ = 0;
			other.data_ = NULL;
		}

		page& operator=(const page& other) {
			count_ = other.count_;
			data_ = other.data_;
			other.count_ = 0;
			other.data_ = NULL;

			return *this;
		}

		~page() {
			if (data_) {
				for (hash_node* i = data_; i < data_ + count_; ++i)
					i->data.~value_type();
				::operator delete(data_);
			}
		}

		inline size_t used() const { return count_; }
		inline size_t free() const { return capacity() - count_; }
		inline size_t capacity() const { return PAGE_SIZE/sizeof(hash_node); }

		inline bool full() const { return free() == 0; }
		inline bool empty() const { return count_ == 0; }

		inline hash_node& front() const { return *data_; }
		inline hash_node& back() const { return *(data_ + count_ - 1); }

		iterator begin() { return data_; }
		const_iterator begin() const { return data_; }

		iterator end() { return data_ + count_; }
		const_iterator end() const { return data_ + count_; }

		hash_node* push(const value_type& v, hash_node* next = NULL) {
			assert(free() > 0);

			new(reinterpret_cast<void*>(&((data_ + count_)->data))) value_type(v);

			(data_ + count_)->next = next;

			return data_ + count_++;
		}

		void pop() {
			assert(count_ > 0);

			data_[count_-1].data.~value_type();
			--count_;
		}

	private:
		mutable int count_;
		mutable hash_node* data_;
	};

	typedef std::vector<page>                page_list;

	typedef std::vector<hash_node*>          hashtable;

private:
	size_t count_;
	hashtable buckets_;
	page_list pages_;

public:
	class iterator;

	class const_iterator {
	public:
		typedef const_iterator self;

	private:
		typedef id_map<I, T, PAGE_SIZE>           map;

		typedef const map*                        const_map_ptr;

		typedef typename map::iterator            iterator;

		typedef typename map::const_pointer       pointer;
		typedef typename map::const_reference     reference;
		typedef typename map::const_hash_node_ptr const_hash_node_ptr;

		const_map_ptr map_;
		const_hash_node_ptr current_;

	public:
		const_iterator() : map_(), current_() {}
		const_iterator(const_map_ptr m) : map_(m), current_() {}
		const_iterator(const_map_ptr m, const_hash_node_ptr c) : map_(m), current_(c) {}
		const_iterator(const iterator& it): map_(it.map_), current_(it.current_) {}

		self& operator++() {
			if (current_->next)
				current_ = current_->next;
			else
				current_ = map_->findfirstafter(current_->data.first);
			return *this;
		}

		self operator++(int) {
			self tmp = *this;
			if (current_->next)
				current_ = current_->next;
			else
				current_ = map_->findfirstafter(current_->data.first);
			return tmp;
		}

		reference operator*() const { return current_->data; }
		pointer operator->() const { return &current_->data; }

		bool operator==(const self& x) const { return x.current_ == current_; }
		bool operator!=(const self& x) const { return x.current_ != current_; }
	};

	class iterator {
		friend class const_iterator;

	public:
		typedef iterator self;

	private:
		typedef id_map<I, T, PAGE_SIZE>          map;

		typedef const map*                       const_map_ptr;

		typedef typename map::pointer            pointer;
		typedef typename map::reference          reference;
		typedef typename map::hash_node_ptr      hash_node_ptr;

	private:
		const_map_ptr map_;
		hash_node_ptr current_;

	public:
		iterator() : map_(), current_() {}
		iterator(const_map_ptr m) : map_(m), current_() {}
		iterator(const_map_ptr m, hash_node_ptr c) : map_(m), current_(c) {}

		self& operator++() {
			if (current_->next)
				current_ = current_->next;
			else
				current_ = map_->findfirstafter(current_->data.first);
			return *this;
		}

		self operator++(int) {
			self tmp = *this;
			if (current_->next)
				current_ = current_->next;
			else
				current_ = map_->findfirstafter(current_->data.first);
			return tmp;
		}

		reference operator*() const { return current_->data; }
		pointer operator->() const { return &current_->data; }

		bool operator==(const self& x) const { return x.current_ == current_; }
		bool operator!=(const self& x) const { return x.current_ != current_; }
	};

public:
	id_map(int nbuckets = 1024) : count_(0), buckets_(nbuckets, NULL) {
	}

	virtual ~id_map() {
	}

	std::pair<iterator, bool> insert(const value_type& v) {
		if (count_ >= buckets_.size())
			rehash(buckets_.size() * 2);

		if (pages_.empty() || pages_.back().full())
			pages_.push_back(page());

		int bucket = v.first & (buckets_.size() - 1);
		buckets_[bucket] = pages_.back().push(v, buckets_[bucket]);

		++count_;

		return std::make_pair(iterator(this, buckets_[bucket]), true);
	}

	/* erases last added element from the map
	 * !! assumes that there's no other way for elements to be erased !!
	 * if some other erase() is added, this should be rewritten
	 */
	void erase_last() {
		assert(!pages_.empty()); /* external invaiant */
		assert(!pages_.back().empty()); /* internal invariant */

		/* remove element from the hash table */
		int bucket = pages_.back().back().data.first & (buckets_.size() - 1);

		assert(buckets_[bucket] == &pages_.back().back());

		buckets_[bucket] = buckets_[bucket]->next;

		/* remove element from the storage */
		pages_.back().pop();
		if (pages_.back().empty())
			pages_.pop_back();

		--count_;
	}

	inline size_t size() const {
		return count_;
	}

	inline bool empty() const {
		return count_ == 0;
	}

	void clear() {
		buckets_.clear();
		pages_.clear();
		count_ = 0;
	}

	iterator find(key_type v) {
		int bucket = v & (buckets_.size() - 1);

		for (hash_node* n = buckets_[bucket]; n; n = n->next)
			if (n->data.first == v)
				return iterator(this, n);

		return end();
	}

	const_iterator find(key_type v) const {
		int bucket = v & (buckets_.size() - 1);

		for (const hash_node* n = buckets_[bucket]; n; n = n->next)
			if (n->data.first == v)
				return const_iterator(this, n);

		return end();
	}

	iterator begin() {
		if (count_ == 0)
			return end();

		return iterator(this, findfirst());
	}

	const_iterator begin() const {
		if (count_ == 0)
			return end();

		return const_iterator(this, findfirst());
	}

	iterator end() {
		return iterator(this, NULL);
	}

	const_iterator end() const {
		return const_iterator(this, NULL);
	}

	void swap(id_map<I, T, PAGE_SIZE>& other) {
		buckets_.swap(other.buckets_);
		pages_.swap(other.pages_);
		std::swap(count_, other.count_);
	}

	void rehash(size_t size) {
		assert(size > 0);
		assert((size & (size - 1)) == 0); // power of two

		hashtable newbuckets(size, NULL);

		for (typename page_list::iterator p = pages_.begin(); p != pages_.end(); ++p) {
			for (typename page::iterator i = p->begin(); i != p->end(); ++i) {
				int bucket = i->data.first & (newbuckets.size() - 1);
				i->next = newbuckets[bucket];
				newbuckets[bucket] = i;
			}
		}

		buckets_.swap(newbuckets);
	}

protected:
	hash_node* findfirst() const {
		for (typename hashtable::const_iterator i = buckets_.begin(); i != buckets_.end(); ++i)
			if (*i != NULL)
				return *i;

		return NULL;
	}

	hash_node* findfirstfor(key_type v) const {
		int bucket = v & (buckets_.size() - 1);

		for (typename hashtable::const_iterator i = buckets_.begin() + bucket; i != buckets_.end(); ++i)
			if (*i != NULL)
				return *i;

		return NULL;
	}

	hash_node* findfirstafter(key_type v) const {
		int bucket = v & (buckets_.size() - 1);

		for (typename hashtable::const_iterator i = buckets_.begin() + bucket + 1; i != buckets_.end(); ++i)
			if (*i != NULL)
				return *i;

		return NULL;
	}
};

#endif
