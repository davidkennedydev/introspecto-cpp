#include <algorithm>
#include <cctype>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <cstdio>
#include <fstream>
#include <ios>
#include <ostream>

std::ofstream reflection_generated(".introspecto_generated.h");

#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/LLVM.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <iostream>

#include <set>

std::vector<std::string> user_declared_files;

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

    if (!IsAngled && FileType == decltype(FileType)::C_User &&
        !FileName.str().ends_with("introspecto.h") &&
        !FileName.str().ends_with("_generated.h")) {
      user_declared_files.push_back(SearchPath.str() + "/" + FileName.str());
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

  bool VisitCXXRecordDecl(clang::CXXRecordDecl *declaration) {
    if (declaration->isThisDeclarationADefinition() &&
        isUserDefined(declaration) && !declaration->getNameAsString().empty() &&
        !declaration->isInExportDeclContext() &&
        !declaration->fields().empty()) {

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
           !declaration->getNameAsString().starts_with("_");
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

// main.cpp
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>

#include <llvm/Support/CommandLine.h>

llvm::cl::OptionCategory MyToolCategory("My Tool Options");

int main(int argc, const char **argv) {

  reflection_generated << "namespace introspecto {\n\n";

  auto OptionsParser =
      clang::tooling::CommonOptionsParser::create(argc, argv, MyToolCategory);
  clang::tooling::ClangTool Tool(OptionsParser->getCompilations(),
                                 OptionsParser->getSourcePathList());

  const int pre_status = Tool.run(
      clang::tooling::newFrontendActionFactory<IncludeFinderAction>().get());

  clang::tooling::ClangTool userFilesTool(OptionsParser->getCompilations(),
                                          user_declared_files);

  const int status = userFilesTool.run(
      clang::tooling::newFrontendActionFactory<ReflectionFrontendAction>()
          .get());

  reflection_generated << "\n}\n\n";
  reflection_generated.close();

  return 1e4 * pre_status + status;
}
