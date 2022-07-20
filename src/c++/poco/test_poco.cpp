/******************************************************************************
 * Copyright 2022  All Rights Reserved.
 ********************************************************/
#include <iostream>
#include <string>

#include <Poco/ClassLibrary.h>   // NOLINT
#include <Poco/ClassLoader.h>    // NOLINT
#include <Poco/SharedLibrary.h>  // NOLINT

class Component {
 public:
  Component() {}
  virtual ~Component() {}
  virtual bool Init() = 0;
  virtual bool Proc() = 0;
};

class RtkComponent : public Component {
 public:
  bool Init() override { return true; }
  bool Proc() override {
    std::cout << "hello rtk component" << std::endl;
    return true;
  }
};

class CameraComponent : public Component {
 public:
  bool Init() override { return true; }
  bool Proc() override {
    std::cout << "hello camera component" << std::endl;
    return true;
  }
};

POCO_BEGIN_MANIFEST(Component)
POCO_EXPORT_CLASS(RtkComponent)
POCO_EXPORT_CLASS(CameraComponent)
POCO_END_MANIFEST

int main(int argc, char* argv[]) {
  if (argc < 3) {
    std::cout << "args not enough" << std::endl;
    std::cout << "example: test_poco library_path classname" << std::endl;
    return -1;
  }
  std::string library_path(argv[1]);
  std::string classname(argv[2]);
  std::cout << "load library:" << library_path
            << " , load classname: " << classname << std::endl;
  Poco::ClassLoader<Component> clc;

  try {
    clc.loadLibrary(library_path);
  } catch (const Poco::Exception e) {
    std::cerr << e.displayText() << std::endl;
    return -1;
  }

  auto* base = clc.create(classname);
  if (base->Init()) {
    base->Proc();
  }
  typeid(base).name();
  return 0;
}
