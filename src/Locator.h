#pragma once

#include <ResourceBase.h>

#include <memory>

class Locator {
public:
    static ResourceBase* getResource();

    static void provide(std::unique_ptr<ResourceBase>&& resource);

private:
    static std::unique_ptr<ResourceBase> resource_;
};
