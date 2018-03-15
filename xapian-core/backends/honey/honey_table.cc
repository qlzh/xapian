/** @file honey_table.cc
 * @brief HoneyTable class
 */
/* Copyright (C) 2017,2018 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */


#include <config.h>

static bool DEBUGGING = false;

#include "honey_table.h"

#include "honey_cursor.h"
#include "stringutils.h"

#include "unicode/description_append.h"
#include <iostream>

using Honey::RootInfo;

using namespace std;

size_t HoneyTable::total_index_size = 0;

void
HoneyTable::create_and_open(int flags_, const RootInfo& root_info)
{
    Assert(!single_file());
    flags = flags_;
    compress_min = root_info.get_compress_min();
    if (read_only) {
	num_entries = root_info.get_num_entries();
	root = root_info.get_root();
	// FIXME: levels
    }
    if (!fh.open(path, read_only))
	throw Xapian::DatabaseOpeningError("Failed to open HoneyTable", errno);
}

void
HoneyTable::open(int flags_, const RootInfo& root_info, honey_revision_number_t)
{
    flags = flags_;
    compress_min = root_info.get_compress_min();
    num_entries = root_info.get_num_entries();
    offset = root_info.get_offset();
    root = root_info.get_root();
    if (!single_file() && !fh.open(path, read_only)) {
	if (!lazy)
	    throw Xapian::DatabaseOpeningError("Failed to open HoneyTable",
					       errno);
    }
    fh.set_pos(offset);
}

void
HoneyTable::add(const std::string& key,
		const char* val,
		size_t val_size,
		bool compressed)
{
    if (!compressed && compress_min > 0 && val_size > compress_min) {
	size_t compressed_size = val_size;
	CompressionStream comp_stream; // FIXME: reuse
	const char* p = comp_stream.compress(val, &compressed_size);
	if (p) {
	    add(key, p, compressed_size, true);
	    return;
	}
    }

    if (read_only)
	throw Xapian::InvalidOperationError("add() on read-only HoneyTable");
    if (key.size() == 0 || key.size() > HONEY_MAX_KEY_LEN)
	throw Xapian::InvalidArgumentError("Invalid key size: " +
					   str(key.size()));
    if (key <= last_key)
	throw Xapian::InvalidOperationError("New key <= previous key");
    off_t index_pos = fh.get_pos();
    if (!last_key.empty()) {
	size_t reuse = common_prefix_length(last_key, key);
	fh.write(static_cast<unsigned char>(reuse));
	fh.write(static_cast<unsigned char>(key.size() - reuse));
	fh.write(key.data() + reuse, key.size() - reuse);
    } else {
	fh.write(static_cast<unsigned char>(key.size()));
	fh.write(key.data(), key.size());
    }
    ++num_entries;
#ifdef SSINDEX_ARRAY
    // For an array index, the index point is right before the complete key.
    if (!last_key.empty()) ++index_pos;
#elif defined SSINDEX_BINARY_CHOP
    // For a binary chop index, the index point is before the key info - the
    // index key must have the same N first bytes as the previous key, where N
    // >= the keep length.
#elif defined SSINDEX_SKIPLIST
    // For a skiplist index, the index provides the full key, so the index
    // point is after the key at the level below.
    index_pos = fh.get_pos();
#else
# error "SSINDEX type not specified"
#endif
    index.maybe_add_entry(key, index_pos);

    // Encode "compressed?" flag in bottom bit.
    // FIXME: Don't do this if a table is uncompressed?  That saves a byte
    // for each item where the extra bit pushes the length up by a byte.
    size_t val_size_enc = (val_size << 1) | compressed;
    std::string val_len;
    pack_uint(val_len, val_size_enc);
    // FIXME: pass together so we can potentially writev() both?
    fh.write(val_len.data(), val_len.size());
    fh.write(val, val_size);
    last_key = key;
}

void
HoneyTable::commit(honey_revision_number_t, RootInfo* root_info)
{
    if (root < 0)
	throw Xapian::InvalidOperationError("root not set");

    root_info->set_level(1); // FIXME: number of index levels
    root_info->set_num_entries(num_entries);
    root_info->set_root_is_fake(false);
    // Not really meaningful.
    root_info->set_sequential(true);
    // offset should already be set.
    root_info->set_root(root);
    // Not really meaningful.
    root_info->set_blocksize(2048);
    // Not really meaningful.
    // root_info->set_free_list(std::string());

    read_only = true;
    fh.rewind(offset);
    last_key = string();
}

bool
HoneyTable::read_key(std::string& key,
		     size_t& val_size,
		     bool& compressed) const
{
    if (DEBUGGING) {
	string desc;
	description_append(desc, key);
	cerr << "HoneyTable::read_key(" << desc << ", ...) for path=" << path << endl;
    }
    if (!read_only) {
	return false;
    }

    AssertRel(fh.get_pos(), >=, offset);
    if (fh.get_pos() >= root) {
	AssertEq(fh.get_pos(), root);
	return false;
    }
    int ch = fh.read();
    if (ch == EOF) return false;

    size_t reuse = 0;
    if (!last_key.empty()) {
	reuse = ch;
	ch = fh.read();
	if (ch == EOF) {
	    throw Xapian::DatabaseError("EOF/error while reading key length",
					errno);
	}
    }
    size_t key_size = ch;
    char buf[256];
    fh.read(buf, key_size);
    key.assign(last_key, 0, reuse);
    key.append(buf, key_size);
    last_key = key;

    if (false) {
	std::string esc;
	description_append(esc, key);
	std::cout << "K:" << esc << std::endl;
    }

    int r;
    {
	// FIXME: rework to take advantage of buffering that's happening anyway?
	char * p = buf;
	for (int i = 0; i < 8; ++i) {
	    int ch2 = fh.read();
	    if (ch2 == EOF) {
		break;
	    }
	    *p++ = ch2;
	    if (ch2 < 128) break;
	}
	r = p - buf;
    }
    const char* p = buf;
    const char* end = p + r;
    if (!unpack_uint(&p, end, &val_size)) {
	throw Xapian::DatabaseError("val_size unpack_uint invalid");
    }
    compressed = val_size & 1;
    val_size >>= 1;
    Assert(p == end);
    return true;
}

void
HoneyTable::read_val(std::string& val, size_t val_size) const
{
    AssertRel(fh.get_pos() + val_size, <=, size_t(root));
    val.resize(val_size);
    fh.read(&(val[0]), val_size);
    if (false) {
	std::string esc;
	description_append(esc, val);
	std::cout << "V:" << esc << std::endl;
    }
}

bool
HoneyTable::get_exact_entry(const std::string& key, std::string* tag) const
{
    if (!read_only) std::abort();
    if (!fh.is_open()) {
	if (fh.was_forced_closed())
	    throw_database_closed();
	return false;
    }
    fh.rewind(root);
    if (rare(key.empty()))
	return false;
    bool exact_match = false;
    bool compressed;
    size_t val_size = 0;
    int index_type = fh.read();
    switch (index_type) {
	case EOF:
	    return false;
	case 0x00: {
	    unsigned char first = key[0] - fh.read();
	    unsigned char range = fh.read();
	    if (first > range)
		return false;
	    fh.skip(first * 4); // FIXME: pointer width
	    off_t jump = fh.read() << 24;
	    jump |= fh.read() << 16;
	    jump |= fh.read() << 8;
	    jump |= fh.read();
	    fh.rewind(jump);
	    // The jump point will be an entirely new key (because it is the
	    // first key with that initial character), and we drop in as if
	    // this was the first key so set last_key to be empty.
	    last_key = string();
	    break;
	}
	case 0x01: {
	    size_t j = fh.read() << 24;
	    j |= fh.read() << 16;
	    j |= fh.read() << 8;
	    j |= fh.read();
	    if (j == 0)
		return false;
	    off_t base = fh.get_pos();
	    char kkey[SSINDEX_BINARY_CHOP_KEY_SIZE];
	    size_t kkey_len = 0;
	    size_t i = 0;
	    while (j - i > 1) {
		size_t k = i + (j - i) / 2;
		fh.set_pos(base + k * (SSINDEX_BINARY_CHOP_KEY_SIZE + 4));
		fh.read(kkey, SSINDEX_BINARY_CHOP_KEY_SIZE);
		kkey_len = 4;
		while (kkey_len > 0 && kkey[kkey_len - 1] == '\0') --kkey_len;
		int r = key.compare(0, SSINDEX_BINARY_CHOP_KEY_SIZE, kkey, kkey_len);
		if (r < 0) {
		    j = k;
		} else {
		    i = k;
		    if (r == 0) {
			break;
		    }
		}
	    }
	    fh.set_pos(base + i * (SSINDEX_BINARY_CHOP_KEY_SIZE + 4));
	    fh.read(kkey, SSINDEX_BINARY_CHOP_KEY_SIZE);
	    kkey_len = 4;
	    while (kkey_len > 0 && kkey[kkey_len - 1] == '\0') --kkey_len;
	    off_t jump = fh.read() << 24;
	    jump |= fh.read() << 16;
	    jump |= fh.read() << 8;
	    jump |= fh.read();
	    fh.rewind(jump);
	    // The jump point is to the first key with prefix kkey, so will
	    // work if we set last key to kkey.  Unless we're jumping to the
	    // start of the table, in which case last_key needs to be empty.
	    last_key.assign(kkey, jump == 0 ? 0 : kkey_len);
	    break;
	}
	case 0x02: {
	    // FIXME: If "close" just seek forwards?  Or consider seeking from
	    // current index pos?
	    // off_t pos = fh.get_pos();
	    string index_key, prev_index_key;
	    make_unsigned<off_t>::type ptr = 0;
	    int cmp0 = 1;
	    while (true) {
		int reuse = fh.read();
		if (reuse == EOF) break;
		int len = fh.read();
		if (len == EOF) abort(); // FIXME
		index_key.resize(reuse + len);
		fh.read(&index_key[reuse], len);

		if (DEBUGGING) {
		    string desc;
		    description_append(desc, index_key);
		    cerr << "Index key: " << desc << endl;
		}

		cmp0 = index_key.compare(key);
		if (cmp0 > 0) {
		    index_key = prev_index_key;
		    break;
		}
		char buf[8];
		char* e = buf;
		while (true) {
		    int b = fh.read();
		    *e++ = b;
		    if ((b & 0x80) == 0) break;
		}
		const char* p = buf;
		if (!unpack_uint(&p, e, &ptr) || p != e) abort(); // FIXME
		if (DEBUGGING) cerr << " -> " << ptr << endl;
		if (cmp0 == 0)
		    break;
		prev_index_key = index_key;
	    }
	    if (DEBUGGING)
		cerr << " cmp0 = " << cmp0 << ", going to " << ptr << endl;
	    fh.set_pos(ptr);

	    if (ptr != 0) {
		last_key = index_key;
		char buf[8];
		int r;
		{
		    // FIXME: rework to take advantage of buffering that's happening anyway?
		    char * p = buf;
		    for (int i = 0; i < 8; ++i) {
			int ch2 = fh.read();
			if (ch2 == EOF) {
			    break;
			}
			*p++ = ch2;
			if (ch2 < 128) break;
		    }
		    r = p - buf;
		}
		const char* p = buf;
		const char* end = p + r;
		if (!unpack_uint(&p, end, &val_size)) {
		    throw Xapian::DatabaseError("val_size unpack_uint invalid");
		}
		compressed = val_size & 1;
		val_size >>= 1;
		Assert(p == end);
	    } else {
		last_key = string();
	    }

	    if (cmp0 == 0) {
		exact_match = true;
		break;
	    }

	    if (DEBUGGING) {
		string desc;
		description_append(desc, last_key);
		cerr << "Dropped to data layer on key: " << desc << endl;
	    }

	    break;
	}
	default:
	    throw Xapian::DatabaseCorruptError("Unknown index type");
    }

    std::string k;
    int cmp;
    if (!exact_match) {
	do {
	    if (val_size) {
		// Skip val data we've not looked at.
		fh.skip(val_size);
		val_size = 0;
	    }
	    if (!read_key(k, val_size, compressed)) return false;
	    cmp = k.compare(key);
	} while (cmp < 0);
	if (cmp > 0) return false;
    }
    if (tag != NULL) {
	if (compressed) {
	    std::string v;
	    read_val(v, val_size);
	    CompressionStream comp_stream;
	    comp_stream.decompress_start();
	    tag->resize(0);
	    if (!comp_stream.decompress_chunk(v.data(), v.size(), *tag)) {
		// Decompression didn't complete.
		abort();
	    }
	} else {
	    read_val(*tag, val_size);
	}
    }
    return true;
}

HoneyCursor*
HoneyTable::cursor_get() const
{
    return new HoneyCursor(fh, root, offset);
}
