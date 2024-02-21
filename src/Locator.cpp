#include <Locator.h>

std::unique_ptr<ResourceBase> Locator::resource_;

ResourceBase* Locator::getResource() {
    return resource_.get();
}

void Locator::provide(std::unique_ptr<ResourceBase>&& resource) {
    resource_ = std::move(resource);
}
