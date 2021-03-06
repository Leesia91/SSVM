// SPDX-License-Identifier: Apache-2.0
//===-- ssvm/common/ast/type.h - type classes definition ------------------===//
//
// Part of the SSVM Project.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the declaration of the type classes: Limit, FunctionType,
/// MemoryType, TableType, and GlobalType.
///
//===----------------------------------------------------------------------===//
#pragma once

#include "base.h"

#include <memory>
#include <vector>

namespace SSVM {
namespace AST {

/// AST Limit node.
class Limit : public Base {
public:
  /// Limit type enumeration class.
  enum class LimitType : uint8_t { HasMin = 0x00, HasMinMax = 0x01 };

  Limit() {}
  Limit(const uint32_t MinVal) : Type(LimitType::HasMin), Min(MinVal) {}
  Limit(const uint32_t MinVal, const uint32_t MaxVal)
      : Type(LimitType::HasMinMax), Min(MinVal), Max(MaxVal) {}

  /// Load binary from file manager.
  ///
  /// Inheritted and overrided from Base.
  /// Read limit type.
  /// Read Min and Max value of this node.
  ///
  /// \param Mgr the file manager reference.
  ///
  /// \returns void when success, ErrMsg when failed.
  Expect<void> loadBinary(FileMgr &Mgr) override;

  /// Getter of having max in limit.
  bool hasMax() const { return Type == LimitType::HasMinMax; }

  /// Getter of min.
  uint32_t getMin() const { return Min; }

  /// Getter of max.
  uint32_t getMax() const { return Max; }

protected:
  /// The node type should be Attr::Type_Limit.
  Attr NodeAttr = Attr::Type_Limit;

private:
  /// \name Data of Limit node.
  /// @{
  LimitType Type;
  uint32_t Min;
  uint32_t Max;
  /// @}
};

/// AST FunctionType node.
class FunctionType : public Base {
public:
  /// Load binary from file manager.
  ///
  /// Inheritted and overrided from Base.
  /// Read value types of parameter list and return list.
  ///
  /// \param Mgr the file manager reference.
  ///
  /// \returns void when success, ErrMsg when failed.
  Expect<void> loadBinary(FileMgr &Mgr) override;

  /// Getter of parameter types vector.
  const std::vector<ValType> &getParamTypes() const { return ParamTypes; }

  /// Getter of return types vector.
  const std::vector<ValType> &getReturnTypes() const { return ReturnTypes; }

protected:
  /// The node type should be Attr::Type_Function.
  Attr NodeAttr = Attr::Type_Function;

private:
  /// \name Data of FunctionType node.
  /// @{
  std::vector<ValType> ParamTypes;
  std::vector<ValType> ReturnTypes;
  /// @}
};

/// AST MemoryType node.
class MemoryType : public Base {
public:
  /// Load binary from file manager.
  ///
  /// Inheritted and overrided from Base.
  /// Read the Limit data of this node.
  ///
  /// \param Mgr the file manager reference.
  ///
  /// \returns void when success, ErrMsg when failed.
  Expect<void> loadBinary(FileMgr &Mgr) override;

  /// Getter of limit.
  const Limit *getLimit() const { return Memory.get(); }

  /// Getter of compiled symbol.
  void *getSymbol() const { return Symbol; }
  /// Setter of compiled symbol.
  void setSymbol(void *S) { Symbol = S; }

protected:
  /// The node type should be Attr::Type_Memory.
  Attr NodeAttr = Attr::Type_Memory;

private:
  /// Data of MemoryType node.
  std::unique_ptr<Limit> Memory;
  void *Symbol = nullptr;
};

/// AST TableType node.
class TableType : public Base {
public:
  /// Load binary from file manager.
  ///
  /// Inheritted and overrided from Base.
  /// Read element type and Limit data.
  ///
  /// \param Mgr the file manager reference.
  ///
  /// \returns void when success, ErrMsg when failed.
  Expect<void> loadBinary(FileMgr &Mgr) override;

  /// Getter of element type.
  ElemType getElementType() const { return Type; }

  /// Getter of limit.
  const Limit *getLimit() const { return Table.get(); }

protected:
  /// The node type should be Attr::Type_Table.
  Attr NodeAttr = Attr::Type_Table;

private:
  /// \name Data of TableType node.
  /// @{
  ElemType Type;
  std::unique_ptr<Limit> Table;
  /// @}
};

/// AST GlobalType node.
class GlobalType : public Base {
public:
  /// Load binary from file manager.
  ///
  /// Inheritted and overrided from Base.
  /// Read value type and mutation.
  ///
  /// \param Mgr the file manager reference.
  ///
  /// \returns void when success, ErrMsg when failed.
  Expect<void> loadBinary(FileMgr &Mgr) override;

  /// Getter of global type.
  ValType getValueType() const { return Type; }

  /// Getter of global mutation.
  ValMut getValueMutation() const { return Mut; }

protected:
  /// The node type should be Attr::Type_Global.
  Attr NodeAttr = Attr::Type_Global;

private:
  /// \name Data of GlobalType node.
  /// @{
  ValType Type;
  ValMut Mut;
  /// @}
};

} // namespace AST
} // namespace SSVM
