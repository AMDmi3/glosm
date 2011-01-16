/*
 * Copyright (C) 2010-2011 Dmitry Marakasov
 *
 * This file is part of glosm.
 *
 * glosm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * glosm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with glosm.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ID_MAP_HH
#define ID_MAP_HH

#include <vector>

/*
 * Container with std::map - like interface that stores
 * <integer, POD> map in a speed and memory-efficient way.
 *
 * Notes:
 * - This container is growing-only - no element deallocation possible
 */

template <typename I, typename T, int BUCKET_COUNT_ORDER = 0, int REHASH_GROWTH_ORDER = 1, int ITEMS_PER_CHUNK = 16*65536>
class id_map {
	static const size_t chunk_size = ITEMS_PER_CHUNK;

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

public:
	struct iterator {
		typedef iterator _self;
		typedef id_map<I, T, BUCKET_COUNT_ORDER, REHASH_GROWTH_ORDER, ITEMS_PER_CHUNK> _map;

		typedef const _map*                       const_map_ptr;

		typedef typename _map::pointer            pointer;
		typedef typename _map::reference          reference;
		typedef typename _map::hash_node_ptr      hash_node_ptr;

		const_map_ptr map;
		hash_node_ptr current;

		iterator() : map(), current() {}
		iterator(const_map_ptr m) : map(m), current() {}
		iterator(const_map_ptr m, hash_node_ptr c) : map(m), current(c) {}

		_self& operator++() {
			if (current->next)
				current = current->next;
			else
				current = map->findfirstafter(current->data.first);
			return *this;
		}

		_self operator++(int) {
			_self tmp = *this;
			if (current->next)
				current = current->next;
			else
				current = map->findfirstafter(current->data.first);
			return tmp;
		}

		reference operator*() const {
			return current->data;
		}

		pointer operator->() const {
			return &current->data;
		}

		bool operator==(const _self& x) const {
			return x.current == current;
		}

		bool operator!=(const _self& x) const {
			return x.current != current;
		}
	};

	struct const_iterator {
		typedef const_iterator _self;
		typedef id_map<I, T, BUCKET_COUNT_ORDER, REHASH_GROWTH_ORDER, ITEMS_PER_CHUNK> _map;

		typedef const _map*                        const_map_ptr;

		typedef typename _map::iterator            iterator;

		typedef typename _map::const_pointer       pointer;
		typedef typename _map::const_reference     reference;
		typedef typename _map::const_hash_node_ptr const_hash_node_ptr;

		const_map_ptr map;
		const_hash_node_ptr current;

		const_iterator() : map(), current() {}
		const_iterator(const_map_ptr m) : map(m) {}
		const_iterator(const_map_ptr m, const_hash_node_ptr c) : map(m), current(c) {}
		const_iterator(const iterator& it): map(it.map), current(it.current) {}

		_self& operator++() {
			if (current->next)
				current = current->next;
			else
				current = map->findfirstafter(current->data.first);
			return *this;
		}

		_self operator++(int) {
			_self tmp = *this;
			if (current->next)
				current = current->next;
			else
				current = map->findfirstafter(current->data.first);
			return tmp;
		}

		reference operator*() const {
			return current->data;
		}

		pointer operator->() const {
			return &current->data;
		}

		bool operator==(const _self& x) const {
			return x.current == current;
		}

		bool operator!=(const _self& x) const {
			return x.current != current;
		}
	};

public:
	typedef std::vector<hash_node*> chunk_list;

public:
	id_map() {
		init();
	}

	virtual ~id_map() {
		deinit();
	}

	std::pair<iterator, bool> insert(const value_type& v) {
		if (REHASH_GROWTH_ORDER > 0 && count > nbuckets * 2)
			rehash(REHASH_GROWTH_ORDER);

		hash_node* t = reinterpret_cast<hash_node*>(alloc());
		new((void*)&t->data)value_type(v);

		/* No checking for existing element done - we assume OSM
		 * data to not have objects with duplicate id's */
		t->next = buckets[v.first & (nbuckets-1)];
		buckets[v.first & (nbuckets-1)] = t;
		count++;

		return std::make_pair(iterator(this, t), true);
	}

	size_t size() const {
		return count;
	}

	void clear() {
		deinit();
		init();
	}

	iterator find(key_type v) {
		for (hash_node* n = buckets[v & (nbuckets-1)]; n; n = n->next)
			if (n->data.first == v)
				return iterator(this, n);

		return end();
	}

	const_iterator find(key_type v) const {
		for (const hash_node* n = buckets[v & (nbuckets-1)]; n; n = n->next)
			if (n->data.first == v)
				return const_iterator(this, n);

		return end();
	}

	iterator begin() {
		if (count == 0)
			return end();

		return iterator(this, findfirst());
	}

	const_iterator begin() const {
		if (count == 0)
			return end();

		return const_iterator(this, findfirst());
	}

	iterator end() {
		return iterator(this, NULL);
	}

	const_iterator end() const {
		return const_iterator(this, NULL);
	}

protected:
	hash_node* findfirst() const {
		for (hash_node** b = buckets; b < buckets + nbuckets; ++b)
			if (*b != NULL)
				return *b;

		return NULL;
	}

	hash_node* findfirstfor(key_type v) const {
		for (hash_node** b = buckets + (v & (nbuckets-1)); b < buckets + nbuckets; ++b)
			if (*b != NULL)
				return *b;

		return NULL;
	}

	hash_node* findfirstafter(key_type v) const {
		for (hash_node** b = buckets + (v & (nbuckets-1)) + 1; b < buckets + nbuckets; ++b)
			if (*b != NULL)
				return *b;

		return NULL;
	}

	hash_node* alloc() {
		if (last_chunk_free == 0) {
			/* chunk filled; allocate new one */
			chunks.push_back(reinterpret_cast<hash_node*>(::operator new(chunk_size*sizeof(hash_node))));
			current_ptr = chunks.back();
			last_chunk_free = chunk_size;
		}
		hash_node* ret = current_ptr;
		current_ptr++;
		last_chunk_free--;
		return ret;
	}

	void rehash(int k) {
		int newnbuckets = nbuckets * (1 << k);

		hash_node** newbuckets = new hash_node*[newnbuckets];
		bzero(newbuckets, newnbuckets * sizeof(hash_node*));

		for (hash_node** b = buckets; b < buckets + nbuckets; ++b) {
			for (hash_node* n = *b; n != NULL; ) {
				hash_node* cur = n;
				n = n->next;
				cur->next = newbuckets[cur->data.first & (newnbuckets-1)];
				newbuckets[cur->data.first & (newnbuckets-1)] = cur;
			}
		}

		nbuckets = newnbuckets;
		delete[] buckets;
		buckets = newbuckets;
	}

	void deinit() {
		for (typename chunk_list::iterator c = chunks.begin(); c != chunks.end(); ++c) {
			/* Call destructors for assigned elements */
			for (hash_node* i = *c; i < ((*c == chunks.back()) ? (*c + chunk_size - last_chunk_free) : (*c + chunk_size)); ++i) {
				i->data.~value_type();
			}
			::operator delete(*c);
		}

		delete[] buckets;
	}

	void init() {
		count = 0;
		last_chunk_free = 0;
		nbuckets = 1 << BUCKET_COUNT_ORDER;
		buckets = new hash_node*[nbuckets];
		bzero(buckets, nbuckets * sizeof(hash_node*));
	}

protected:
	/* hash table */
	size_t nbuckets; /* always power of 2, so nbuckets-1 is bitmask for hashing */
	hash_node** buckets;
	size_t count; /* XXX: superfluous - may be derived from nchunks and urrent_ptr */

	/* memory pool */
	chunk_list chunks;
	size_t last_chunk_free;
	hash_node* current_ptr; /* XXX: superfluous - either last_chunk free or current_ptr should be removed */
};

#endif
