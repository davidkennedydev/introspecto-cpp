#include <cctype>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <fstream>
#include <ostream>

std::ofstream reflection_generated(".introspecto_generated.h");

class ReflectionASTVisitor
    : public clang::RecursiveASTVisitor<ReflectionASTVisitor> {
  std::ostream &generated;

public:
  ReflectionASTVisitor(std::ostream &out) : generated(out){};

  bool VisitCXXRecordDecl(clang::CXXRecordDecl *declaration) {
    if (declaration->isThisDeclarationADefinition() &&
        !declaration->isInStdNamespace() &&
        std::isupper(declaration->getNameAsString().front()) &&
        !declaration->getNameAsString().empty() &&
        !declaration->getNameAsString().starts_with("_") &&
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
  const int status = Tool.run(
      clang::tooling::newFrontendActionFactory<ReflectionFrontendAction>()
          .get());

  reflection_generated << "\n}\n\n";
  reflection_generated.close();

  return status;
}
