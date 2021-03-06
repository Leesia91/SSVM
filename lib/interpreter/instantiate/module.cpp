// SPDX-License-Identifier: Apache-2.0
#include "common/ast/module.h"
#include "common/ast/section.h"
#include "interpreter/interpreter.h"
#include "runtime/instance/module.h"

namespace SSVM {
namespace Interpreter {

/// Instantiate module instance. See "include/executor/Interpreter.h".
Expect<void> Interpreter::instantiate(Runtime::StoreManager &StoreMgr,
                                      const AST::Module &Mod,
                                      const std::string &Name) {
  /// Reset store manager, stack manager, and instruction provider.
  StoreMgr.reset();
  StackMgr.reset();
  InstrPdr.reset();

  /// Check is module name duplicated.
  if (auto Res = StoreMgr.findModule(Name)) {
    return Unexpect(ErrCode::ModuleNameConflict);
  }
  auto NewModInst = std::make_unique<Runtime::Instance::ModuleInstance>(Name);

  /// Insert the module instance to store manager and retrieve instance.
  uint32_t ModInstAddr;
  if (InsMode == InstantiateMode::Instantiate) {
    ModInstAddr = StoreMgr.pushModule(NewModInst);
  } else {
    ModInstAddr = StoreMgr.importModule(NewModInst);
  }
  auto *ModInst = *StoreMgr.getModule(ModInstAddr);

  /// Instantiate Function Types in Module Instance. (TypeSec)
  const AST::TypeSection *TypeSec = Mod.getTypeSection();
  if (TypeSec != nullptr) {
    auto &FuncTypes = TypeSec->getContent();
    for (auto &FuncType : FuncTypes) {
      /// Copy param and return lists to module instance.
      ModInst->addFuncType(FuncType->getParamTypes(),
                           FuncType->getReturnTypes());
    }
  }

  /// Instantiate ImportSection and do import matching. (ImportSec)
  const AST::ImportSection *ImportSec = Mod.getImportSection();
  if (ImportSec != nullptr) {
    if (auto Res = instantiate(StoreMgr, *ModInst, *ImportSec); !Res) {
      return Unexpect(Res);
    }
  }

  /// Instantiate Functions in module. (FuncionSec, CodeSec)
  const AST::FunctionSection *FuncSec = Mod.getFunctionSection();
  const AST::CodeSection *CodeSec = Mod.getCodeSection();
  if (FuncSec != nullptr && CodeSec != nullptr) {
    if (auto Res = instantiate(StoreMgr, *ModInst, *FuncSec, *CodeSec); !Res) {
      return Unexpect(Res);
    }
  }

  /// Instantiate GlobalSection (GlobalSec)
  const AST::GlobalSection *GlobSec = Mod.getGlobalSection();
  if (GlobSec != nullptr) {
    if (auto Res = instantiate(StoreMgr, *ModInst, *GlobSec); !Res) {
      return Unexpect(Res);
    }
  }

  /// Instantiate TableSection (TableSec)
  const AST::TableSection *TabSec = Mod.getTableSection();
  if (TabSec != nullptr) {
    if (auto Res = instantiate(StoreMgr, *ModInst, *TabSec); !Res) {
      return Unexpect(Res);
    }
  }

  /// Instantiate MemorySection (MemorySec)
  const AST::MemorySection *MemSec = Mod.getMemorySection();
  if (MemSec != nullptr) {
    if (auto Res = instantiate(StoreMgr, *ModInst, *MemSec); !Res) {
      return Unexpect(Res);
    }
  }

  /// Initialize the tables and memories
  /// Make a new frame {ModInst, locals:none} and push
  StackMgr.pushFrame(ModInst->Addr, /// Module address
                     0,             /// Arity
                     0              /// Coarity
  );
  std::vector<uint32_t> ElemOffsets, DataOffsets;

  /// Resolve offset list of element section.
  const AST::ElementSection *ElemSec = Mod.getElementSection();
  if (ElemSec != nullptr) {
    if (auto Res = resolveExpression(StoreMgr, *ModInst, *ElemSec)) {
      ElemOffsets = std::move(*Res);
    } else {
      return Unexpect(Res);
    }
  }

  /// Resolve offset list of data section.
  const AST::DataSection *DataSec = Mod.getDataSection();
  if (DataSec != nullptr) {
    if (auto Res = resolveExpression(StoreMgr, *ModInst, *DataSec)) {
      DataOffsets = std::move(*Res);
    } else {
      return Unexpect(Res);
    }
  }

  /// Pop Frame.
  StackMgr.popFrame();

  /// Instantiate initialization of table instances (ElemSec)
  if (ElemSec != nullptr) {
    if (auto Res = instantiate(StoreMgr, *ModInst, *ElemSec, ElemOffsets);
        !Res) {
      return Unexpect(Res);
    }
  }

  /// Instantiate initialization of memory instances (DataSec)
  if (DataSec != nullptr) {
    if (auto Res = instantiate(StoreMgr, *ModInst, *DataSec, DataOffsets);
        !Res) {
      return Unexpect(Res);
    }
  }

  /// Instantiate ExportSection (ExportSec)
  const AST::ExportSection *ExportSec = Mod.getExportSection();
  if (ExportSec != nullptr) {
    if (auto Res = instantiate(StoreMgr, *ModInst, *ExportSec); !Res) {
      return Unexpect(Res);
    }
  }

  /// Call Ctor for compiled module
  if (auto CtorFunc = Mod.getCtor(); CtorFunc != nullptr) {
    CtorFunc(Interpreter::trapProxy, Interpreter::callProxy,
             Interpreter::memGrowProxy, Interpreter::memSizeProxy);
  }

  /// Instantiate StartSection (StartSec)
  const AST::StartSection *StartSec = Mod.getStartSection();
  if (StartSec != nullptr) {
    /// Get the module instance from ID.
    ModInst->setStartIdx(StartSec->getContent());

    /// Get function instance.
    const uint32_t Addr = *ModInst->getStartAddr();
    const auto *FuncInst = *StoreMgr.getFunction(Addr);

    /// Call runFunction.
    return runFunction(StoreMgr, *FuncInst, {});
  }
  return {};
}

} // namespace Interpreter
} // namespace SSVM
