//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <algorithm>

#include "compiler/translator/TranslatorMetalDirect/SeparateCompoundStructDeclarations.h"
#include "compiler/translator/tree_ops/SeparateDeclarations.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/TranslatorMetalDirect/AstHelpers.h"

using namespace sh;

////////////////////////////////////////////////////////////////////////////////

namespace
{

class Separator : public TIntermTraverser
{
  public:
    std::unordered_map<int, TIntermSymbol *> replacementMap;
    Separator(TSymbolTable &symbolTable, IdGen &idGen) : TIntermTraverser(false, false, true, &symbolTable),
        mIdGen(idGen)
    {}
    IdGen &mIdGen;
    bool visitDeclaration(Visit, TIntermDeclaration *declNode) override
    {
        ASSERT(declNode->getChildCount() == 1);
        Declaration declaration = ViewDeclaration(*declNode);

        const TVariable &var        = declaration.symbol.variable();
        const TType &type           = var.getType();
        const SymbolType symbolType = var.symbolType();
        if (type.isStructSpecifier() && symbolType != SymbolType::Empty)
        {
            const TStructure *structure = type.getStruct();
            TVariable *structVar = nullptr;
            TType * instanceType = nullptr;
            //Name unnamed inline structs
            if(structure->symbolType() == SymbolType::Empty)
            {
                const TStructure *structDefn = new TStructure(mSymbolTable, mIdGen.createNewName().rawName(), &structure->fields(), SymbolType::AngleInternal);
                structVar = new TVariable(mSymbolTable, ImmutableString(""),
                                         new TType(structDefn, true), SymbolType::Empty);
                instanceType = new TType(structDefn, false);
            }
            else
            {
                structVar             = new TVariable(mSymbolTable, ImmutableString(""),
                                                new TType(structure, true), SymbolType::Empty);
                instanceType = new TType(structure, false);
            }
            instanceType->setQualifier(type.getQualifier());
            auto *instanceVar = new TVariable(mSymbolTable, var.name(), instanceType,
                                              symbolType, var.extension());

            TIntermSequence replacements;
            replacements.push_back(new TIntermSymbol(structVar));

            TIntermSymbol * instanceSymbol = new TIntermSymbol(instanceVar);
            TIntermNode * instanceReplacement = instanceSymbol;
            if(declaration.initExpr)
            {
                instanceReplacement = new TIntermBinary(EOpInitialize, instanceSymbol, declaration.initExpr);
            }
            replacements.push_back(instanceReplacement);

            replacementMap[declaration.symbol.uniqueId().get()] = instanceSymbol;
            mMultiReplacements.push_back(
                NodeReplaceWithMultipleEntry(declNode, declNode->getChildNode(0), std::move(replacements)));
        }

        return false;
    }
    
    void visitSymbol(TIntermSymbol *decl) override
    {
        auto symbol = replacementMap.find(decl->uniqueId().get());
        if(symbol != replacementMap.end())
        {
            queueReplacement(symbol->second->deepCopy(), OriginalNode::IS_DROPPED);
        }
    }
};

}  // anonymous namespace

////////////////////////////////////////////////////////////////////////////////

bool sh::SeparateCompoundStructDeclarations(TCompiler &compiler, IdGen &idGen, TIntermBlock &root)
{
    Separator separator(compiler.getSymbolTable(), idGen);
    root.traverse(&separator);
    if (!separator.updateTree(&compiler, &root))
    {
        return false;
    }

    if (!SeparateDeclarations(&compiler, &root))
    {
        return false;
    }

    return true;
}
