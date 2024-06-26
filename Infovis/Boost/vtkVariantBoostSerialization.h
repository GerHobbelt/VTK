// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-FileCopyrightText: Copyright (C) 2008 The Trustees of Indiana University.
// SPDX-License-Identifier: BSD-3-Clause AND BSL-1.0
/**
 * @class   vtkVariantBoostSerialization
 * @brief   Serialization support for
 * vtkVariant and vtkVariantArray using the Boost.Serialization
 * library.
 *
 *
 * The header includes the templates required to serialize the
 * vtkVariant and vtkVariantArray with the Boost.Serialization
 * library. Just including the header suffices to get serialization
 * support; no other action is needed.
 */

#ifndef vtkVariantBoostSerialization_h
#define vtkVariantBoostSerialization_h

#include "vtkSetGet.h"
#include "vtkType.h"
#include "vtkVariant.h"
#include "vtkVariantArray.h"

// This include fixes header-ordering issues in Boost.Serialization
// prior to Boost 1.35.0.
#include <boost/archive/binary_oarchive.hpp>

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/extended_type_info_no_rtti.hpp>
#include <boost/serialization/split_free.hpp>

VTK_ABI_NAMESPACE_BEGIN

//----------------------------------------------------------------------------
// vtkStdString serialization code
//----------------------------------------------------------------------------
template <typename Archiver>
void serialize(Archiver& ar, vtkStdString& str, const unsigned int vtkNotUsed(version))
{
  ar& boost::serialization::base_object<std::string>(str);
}

//----------------------------------------------------------------------------

template <typename Archiver>
void save(Archiver& ar, const std::string& str, const unsigned int vtkNotUsed(version))
{
  ar& str;
}

template <typename Archiver>
void load(Archiver& ar, std::string& str, const unsigned int vtkNotUsed(version))
{
  std::string utf8;
  ar& utf8;
  str = utf8;
}

//----------------------------------------------------------------------------
// vtkVariant serialization code
//----------------------------------------------------------------------------

template <typename Archiver>
void save(Archiver& ar, const vtkVariant& variant, const unsigned int vtkNotUsed(version))
{
  if (!variant.IsValid())
  {
    char null = 0;
    ar& null;
    return;
  }

  // Output the type
  char Type = variant.GetType();
  ar& Type;

  // Output the value
#define VTK_VARIANT_SAVE(Value, Type, Function)                                                    \
  case Value:                                                                                      \
  {                                                                                                \
    Type value = variant.Function();                                                               \
    ar& value;                                                                                     \
  }                                                                                                \
    return

  switch (Type)
  {
    VTK_VARIANT_SAVE(VTK_STRING, vtkStdString, ToString);
    VTK_VARIANT_SAVE(VTK_FLOAT, float, ToFloat);
    VTK_VARIANT_SAVE(VTK_DOUBLE, double, ToDouble);
    VTK_VARIANT_SAVE(VTK_CHAR, char, ToChar);
    VTK_VARIANT_SAVE(VTK_UNSIGNED_CHAR, unsigned char, ToUnsignedChar);
    VTK_VARIANT_SAVE(VTK_SHORT, short, ToShort);
    VTK_VARIANT_SAVE(VTK_UNSIGNED_SHORT, unsigned short, ToUnsignedShort);
    VTK_VARIANT_SAVE(VTK_INT, int, ToInt);
    VTK_VARIANT_SAVE(VTK_UNSIGNED_INT, unsigned int, ToUnsignedInt);
    VTK_VARIANT_SAVE(VTK_LONG, long, ToLong);
    VTK_VARIANT_SAVE(VTK_UNSIGNED_LONG, unsigned long, ToUnsignedLong);
    VTK_VARIANT_SAVE(VTK_LONG_LONG, long long, ToLongLong);
    VTK_VARIANT_SAVE(VTK_UNSIGNED_LONG_LONG, unsigned long long, ToUnsignedLongLong);
    default:
      cerr << "cannot serialize variant with type " << variant.GetType() << '\n';
  }
#undef VTK_VARIANT_SAVE
}

template <typename Archiver>
void load(Archiver& ar, vtkVariant& variant, const unsigned int vtkNotUsed(version))
{
  char Type;
  ar& Type;

#define VTK_VARIANT_LOAD(Value, Type)                                                              \
  case Value:                                                                                      \
  {                                                                                                \
    Type value;                                                                                    \
    ar& value;                                                                                     \
    variant = vtkVariant(value);                                                                   \
  }                                                                                                \
    return

  switch (Type)
  {
    case 0:
      variant = vtkVariant();
      return;
      VTK_VARIANT_LOAD(VTK_STRING, vtkStdString);
      VTK_VARIANT_LOAD(VTK_FLOAT, float);
      VTK_VARIANT_LOAD(VTK_DOUBLE, double);
      VTK_VARIANT_LOAD(VTK_CHAR, char);
      VTK_VARIANT_LOAD(VTK_UNSIGNED_CHAR, unsigned char);
      VTK_VARIANT_LOAD(VTK_SHORT, short);
      VTK_VARIANT_LOAD(VTK_UNSIGNED_SHORT, unsigned short);
      VTK_VARIANT_LOAD(VTK_INT, int);
      VTK_VARIANT_LOAD(VTK_UNSIGNED_INT, unsigned int);
      VTK_VARIANT_LOAD(VTK_LONG, long);
      VTK_VARIANT_LOAD(VTK_UNSIGNED_LONG, unsigned long);
      VTK_VARIANT_LOAD(VTK_LONG_LONG, long long);
      VTK_VARIANT_LOAD(VTK_UNSIGNED_LONG_LONG, unsigned long long);
    default:
      cerr << "cannot deserialize variant with type " << static_cast<unsigned int>(Type) << '\n';
      variant = vtkVariant();
  }
#undef VTK_VARIANT_LOAD
}

VTK_ABI_NAMESPACE_END

BOOST_SERIALIZATION_SPLIT_FREE(vtkVariant)

VTK_ABI_NAMESPACE_BEGIN

//----------------------------------------------------------------------------
// vtkVariantArray serialization code
//----------------------------------------------------------------------------

template <typename Archiver>
void save(Archiver& ar, const vtkVariantArray& c_array, const unsigned int vtkNotUsed(version))
{
  vtkVariantArray& array = const_cast<vtkVariantArray&>(c_array);

  // Array name
  vtkStdString name;
  if (array.GetName() != nullptr)
    name = array.GetName();
  ar& name;

  // Array data
  vtkIdType n = array.GetNumberOfTuples();
  ar& n;
  for (vtkIdType i = 0; i < n; ++i)
  {
    ar& array.GetValue(i);
  }
}

template <typename Archiver>
void load(Archiver& ar, vtkVariantArray& array, const unsigned int vtkNotUsed(version))
{
  // Array name
  vtkStdString name;
  ar& name;
  array.SetName(name.c_str());

  if (name.empty())
  {
    array.SetName(nullptr);
  }
  else
  {
    array.SetName(name.c_str());
  }

  // Array data
  vtkIdType n;
  ar& n;
  array.SetNumberOfTuples(n);
  vtkVariant value;
  for (vtkIdType i = 0; i < n; ++i)
  {
    ar& value;
    array.SetValue(i, value);
  }
}

VTK_ABI_NAMESPACE_END

BOOST_SERIALIZATION_SPLIT_FREE(vtkVariantArray)

#endif
// VTK-HeaderTest-Exclude: vtkVariantBoostSerialization.h
