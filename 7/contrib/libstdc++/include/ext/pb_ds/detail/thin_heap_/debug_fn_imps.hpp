// -*- C++ -*-

// Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the terms
// of the GNU General Public License as published by the Free Software
// Foundation; either version 2, or (at your option) any later
// version.

// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this library; see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330, Boston,
// MA 02111-1307, USA.

// As a special exception, you may use this file as part of a free
// software library without restriction.  Specifically, if other files
// instantiate templates or use macros or inline functions from this
// file, or you compile this file and link it with other files to
// produce an executable, this file does not by itself cause the
// resulting executable to be covered by the GNU General Public
// License.  This exception does not however invalidate any other
// reasons why the executable file might be covered by the GNU General
// Public License.

// Copyright (C) 2004 Ami Tavory and Vladimir Dreizin, IBM-HRL.

// Permission to use, copy, modify, sell, and distribute this software
// is hereby granted without fee, provided that the above copyright
// notice appears in all copies, and that both that copyright notice
// and this permission notice appear in supporting documentation. None
// of the above authors, nor IBM Haifa Research Laboratories, make any
// representation about the suitability of this software for any
// purpose. It is provided "as is" without express or implied
// warranty.

/**
 * @file debug_fn_imps.hpp
 * Contains an implementation for thin_heap_.
 */

#ifdef _GLIBCXX_DEBUG

PB_DS_CLASS_T_DEC
void
PB_DS_CLASS_C_DEC::
assert_valid() const
{
  base_type::assert_valid();
  assert_node_consistent(base_type::m_p_root, true);
  assert_max();
  assert_aux_null();
}

PB_DS_CLASS_T_DEC
void
PB_DS_CLASS_C_DEC::
assert_aux_null() const
{
  for (size_type i = 0; i < max_rank; ++i)
    _GLIBCXX_DEBUG_ASSERT(m_a_aux[i] == NULL);
}

PB_DS_CLASS_T_DEC
void
PB_DS_CLASS_C_DEC::
assert_max() const
{
  if (m_p_max == NULL)
    {
      _GLIBCXX_DEBUG_ASSERT(base_type::empty());
      return;
    }

  _GLIBCXX_DEBUG_ASSERT(!base_type::empty());
  _GLIBCXX_DEBUG_ASSERT(base_type::parent(m_p_max) == NULL);
  _GLIBCXX_DEBUG_ASSERT(m_p_max->m_p_prev_or_parent == NULL);
  for (const_iterator it = base_type::begin(); it != base_type::end(); ++it)
    _GLIBCXX_DEBUG_ASSERT(!Cmp_Fn::operator()(m_p_max->m_value, it.m_p_nd->m_value));
}

PB_DS_CLASS_T_DEC
void
PB_DS_CLASS_C_DEC::
assert_node_consistent(const_node_pointer p_nd, bool root) const
{
  base_type::assert_node_consistent(p_nd, root);
  if (p_nd == NULL)
    return;

  assert_node_consistent(p_nd->m_p_next_sibling, root);
  assert_node_consistent(p_nd->m_p_l_child, false);
  if (!root)
    {
      if (p_nd->m_metadata == 0)
	_GLIBCXX_DEBUG_ASSERT(p_nd->m_p_next_sibling == NULL);
      else
	_GLIBCXX_DEBUG_ASSERT(p_nd->m_metadata == p_nd->m_p_next_sibling->m_metadata + 1);
    }

  if (p_nd->m_p_l_child != NULL)
    _GLIBCXX_DEBUG_ASSERT(p_nd->m_p_l_child->m_metadata + 1 == base_type::degree(p_nd));

  const bool unmarked_valid =(p_nd->m_p_l_child == NULL&&  p_nd->m_metadata == 0) ||(p_nd->m_p_l_child != NULL&&  p_nd->m_metadata == p_nd->m_p_l_child->m_metadata + 1);

  const bool marked_valid =(p_nd->m_p_l_child == NULL&&  p_nd->m_metadata == 1) ||(p_nd->m_p_l_child != NULL&&  p_nd->m_metadata == p_nd->m_p_l_child->m_metadata + 2);

  _GLIBCXX_DEBUG_ASSERT(unmarked_valid || marked_valid);
  if (root)
    _GLIBCXX_DEBUG_ASSERT(unmarked_valid);
}

#endif 
