#pragma once

class OffsetView;

class LodRenderer {
public:
    virtual ~LodRenderer() noexcept = default;
    // FIXME should be private nonvirtual? probably should add virtual to other derived classes that missed it.
    LodRenderer(const LodRenderer& rhs) = default;
    LodRenderer& operator=(const LodRenderer& rhs) = default;

    // FIXME include some helper functions to abstract functionality from Board and SubBoard, maybe skip the virtual methods.
    virtual void setRenderArea(const OffsetView& offsetView, float zoom) = 0;
    int getCurrentLod() const;

    // FIXME define EMPTY_CHUNK_COORDS in here?

private:
    int currentLod_;
};
