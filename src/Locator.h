#pragma once

#include <ResourceBase.h>

#include <memory>

/**
 * Common access point for services (for the service locator pattern).
 */
class Locator {
public:
    static ResourceBase* getResource();

    static void provide(std::unique_ptr<ResourceBase>&& resource);

private:
    static std::unique_ptr<ResourceBase> resource_;
};
