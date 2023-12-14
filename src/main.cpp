#include <algorithm>
#include <cctype>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Type.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <cstdio>
#include <fstream>
#include <ios>
#include <iterator>
#include <ostream>
#include <string_view>

std::ofstream reflection_generated(".introspecto_generated.h");

#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/LLVM.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <iostream>

#include <unordered_set>

std::unordered_set<std::string> user_declared_files;
std::unordered_set<std::string> declared_symbols;

std::set<std::string> dependency_files;
std::set<std::string> dependency_symbols;

class IncludeFinder : public clang::PPCallbacks {
public:
  void InclusionDirective(clang::SourceLocation HashLoc,
                          const clang::Token &IncludeTok,
                          clang::StringRef FileName, bool IsAngled,
                          clang::CharSourceRange FilenameRange,
                          clang::OptionalFileEntryRef File,
                          clang::StringRef SearchPath,
                          clang::StringRef RelativePath,
                          const clang::Module *Imported,
                          clang::SrcMgr::CharacteristicKind FileType) override {

    if (IsAngled) {
      dependency_files.insert(FileName.str());
    } else if (FileType == decltype(FileType)::C_User &&
        FileName.str().ends_with("hpp") && // XXX files terminated just with h,
                                           // fail to find c++ headers
        !FileName.str().ends_with("introspecto.h") &&
        !FileName.str().ends_with("_generated.h")) {
      user_declared_files.insert(SearchPath.str() + "/" + FileName.str());
      std::cout << "User file declaration: " << FileName.str() << "\n";
    }
  }
};

class IncludeFinderAction : public clang::PreprocessOnlyAction {
protected:
  void ExecuteAction() override {
    clang::Preprocessor &PP = getCompilerInstance().getPreprocessor();
    PP.addPPCallbacks(std::make_unique<IncludeFinder>());
    clang::PreprocessOnlyAction::ExecuteAction();
  }
};

class ReflectionASTVisitor
    : public clang::RecursiveASTVisitor<ReflectionASTVisitor> {
  std::ostream &generated;

public:
  ReflectionASTVisitor(std::ostream &out) : generated(out){};

  virtual bool VisitCXXRecordDecl(clang::CXXRecordDecl *declaration) {
    if (declaration->isThisDeclarationADefinition() &&
        isUserDefined(declaration) && !declaration->getNameAsString().empty() &&
        !declaration->isInExportDeclContext() &&
        !declaration->isModulePrivate() && !declaration->fields().empty()) {

      std::string userType = declaration->getQualifiedNameAsString();

      generated
          << "template <> class Introspect<" << userType << "> {\n"
          << "  " << userType << " &instance;\n"
          << "public:\n\n"
             "  constexpr Introspect("
          << userType
          << " &instance)"
             ": instance(instance) {}\n\n"
             "  constexpr void foreachField(FieldVisitor auto &&apply) {\n";

      for (auto field : declaration->fields()) {
        generated << "    apply(\"" << field->getNameAsString()
                  << "\", instance." << field->getNameAsString() << ");\n";
      }

      generated << "  }\n"
                   "};\n\n";
    }
    return true;
  }

  static bool isUserDefined(clang::CXXRecordDecl *declaration) {
    return declaration->getTranslationUnitDecl() ==
               declaration->getDeclContext() &&
           !declaration->isInStdNamespace() &&
           !declaration->getNameAsString().starts_with("_") &&
           user_declared_files.contains(
               declaration->getASTContext()
                   .getSourceManager()
                   .getPresumedLoc(declaration->getLocation())
                   .getFilename());
  }

  static bool isDeclared(const std::string& symbolName) {
    return declared_symbols.contains(symbolName);
  }

  std::string get_fullname(clang::NamedDecl *decl) {
    std::string name = decl->getNameAsString();
    clang::DeclContext* context = decl->getDeclContext();
    while (context && isa<clang::NamespaceDecl>(context)) {
      auto* namespaceDecl = cast<clang::NamespaceDecl>(context);
      name = namespaceDecl->getNameAsString() + "::" + name;
      context = context->getParent();
    }
    return name;
  }

  virtual void HandleTranslationUnit(clang::ASTContext &Context) {
    for (clang::Decl *Decl : Context.getTranslationUnitDecl()->decls()) {
      if (auto *NamedDecl = dyn_cast<clang::NamedDecl>(Decl)) {
        ::declared_symbols.insert(get_fullname(NamedDecl));
      }
    }
  }

  virtual bool VisitCallExpr(clang::CallExpr *CallExpr) {
    clang::Expr* callee = CallExpr->getCallee()->IgnoreParenCasts();
    if (isa<clang::DeclRefExpr>(callee)) {
      auto* decl_ref = cast<clang::DeclRefExpr>(callee);
      auto* named_decl = decl_ref->getDecl();
      if (named_decl) {
        if (auto name = get_fullname(named_decl);
          !isDeclared(name)
          && named_decl->isUsed()
          && named_decl->isExternallyDeclarable()
          && !named_decl->isInStdNamespace()
          && !named_decl->isImplicit()
          )
          dependency_symbols.insert(name);
        }
    }
    return true;
  }

};

#include <clang/AST/ASTConsumer.h>

class ReflectionASTConsumer : public clang::ASTConsumer {
public:
  ReflectionASTConsumer(std::ofstream &out)
      : visitor(out), clang::ASTConsumer() {}

  virtual void HandleTranslationUnit(clang::ASTContext &Context) override {
    visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  ReflectionASTVisitor visitor;
};

// ReflectionFrontendAction.h
#include <clang/Frontend/FrontendActions.h>

class ReflectionFrontendAction : public clang::ASTFrontendAction {
protected:
  virtual std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    std::cout << "Analysing file: " << InFile.str() << std::endl;
    return std::unique_ptr<clang::ASTConsumer>(
        new ReflectionASTConsumer(::reflection_generated));
  }
};

void GenerateDependencyInfo() {

  reflection_generated << "  namespace dependency {\n";

  auto generate_list = [](std::string &&name,
                          const std::set<std::string> content) {
    reflection_generated << "    constexpr const char* const " << name << "[] = ";
    bool first = true;
    for (std::string_view element : content) {
      reflection_generated << (first ? '{' : ',') << "\n      \"" << element << '"';
      first = false;
    }
    reflection_generated << "\n    };\n\n";
  };

  generate_list("includes", dependency_files);
  generate_list("symbols", dependency_symbols);

  reflection_generated << "  }\n\n";
}

// main.cpp
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>

#include <llvm/Support/CommandLine.h>

llvm::cl::OptionCategory MyToolCategory("My Tool Options");

int main(int argc, const char **argv) {

  reflection_generated << "#pragma once\n\n"
                       << "namespace introspecto {\n\n";

  auto OptionsParser =
      clang::tooling::CommonOptionsParser::create(argc, argv, MyToolCategory);
  clang::tooling::ClangTool Tool(OptionsParser->getCompilations(),
                                 OptionsParser->getSourcePathList());

  int status = Tool.run(
      clang::tooling::newFrontendActionFactory<IncludeFinderAction>().get());

  status += Tool.run(
      clang::tooling::newFrontendActionFactory<ReflectionFrontendAction>()
          .get());

  GenerateDependencyInfo();

  reflection_generated << "}\n\n";
  reflection_generated.close();

  return status;
}
