#include <Board.h>
#include <DebugScreen.h>
#include <LegacyFileFormat.h>
#include <OffsetView.h>
#include <RegionFileFormat.h>
#include <ResourceManager.h>
#include <Tile.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <utility>

sf::Texture* Board::tilesetGrid_;
sf::Texture* Board::tilesetNoGrid_;
unsigned int Board::tileWidth_;

void clampImageToEdge(sf::Image& image, const sf::Vector2u& topLeft, const sf::Vector2u& bottomRight, const sf::Vector2u& borderSize) {
    sf::Vector2u borderTopLeft(topLeft - borderSize), borderBottomRight(bottomRight + borderSize);
    for (unsigned int y = borderTopLeft.y; y < borderBottomRight.y; ++y) {
        for (unsigned int x = borderTopLeft.x; x < borderBottomRight.x; ++x) {
            if (x < topLeft.x || x >= bottomRight.x || y < topLeft.y || y >= bottomRight.y) {
                unsigned int xTarget = std::min(std::max(x, topLeft.x), bottomRight.x - 1);
                unsigned int yTarget = std::min(std::max(y, topLeft.y), bottomRight.y - 1);
                image.setPixel(x, y, image.getPixel(xTarget, yTarget));
            }
        }
    }
}

void loadTileset(const fs::path& filename, sf::Texture* target, unsigned int tileWidth) {
    sf::Image tileset;
    if (!tileset.loadFromFile(filename.string())) {
        throw std::runtime_error("\"" + filename.string() + "\": unable to load texture file.");
    }
    target->create(tileset.getSize().x * 2, tileset.getSize().y * 4);
    sf::Image fullTileset;
    fullTileset.create(tileset.getSize().x * 2, tileset.getSize().y * 2, sf::Color::Red);

    for (unsigned int y = 0; y < tileset.getSize().y; y += tileWidth) {
        for (unsigned int x = 0; x < tileset.getSize().x; x += tileWidth) {
            sf::Vector2u tileTopLeft(x * 2 + tileWidth / 2, y * 2 + tileWidth / 2);
            fullTileset.copy(tileset, tileTopLeft.x, tileTopLeft.y, sf::IntRect(x, y, tileWidth, tileWidth));
            clampImageToEdge(fullTileset, tileTopLeft, tileTopLeft + sf::Vector2u(tileWidth, tileWidth), sf::Vector2u(tileWidth / 2, tileWidth / 2));
        }
    }
    target->update(fullTileset, 0, 0);

    for (unsigned int y = 0; y < fullTileset.getSize().y; ++y) {
        for (unsigned int x = 0; x < fullTileset.getSize().x; ++x) {
            fullTileset.setPixel(x, y, fullTileset.getPixel(x, y) + sf::Color(100, 100, 100));
        }
    }
    target->update(fullTileset, 0, tileset.getSize().y * 2);
    spdlog::debug("Built tileset texture with size {} by {}.", target->getSize().x, target->getSize().y);
}

void Board::setupTextures(ResourceManager& resource, const fs::path& filenameGrid, const fs::path& filenameNoGrid, unsigned int tileWidth) {
    tileWidth_ = tileWidth;

    tilesetGrid_ = &resource.getTexture(filenameGrid, true);
    loadTileset(filenameGrid, tilesetGrid_, tileWidth);
    tilesetGrid_->setSmooth(true);
    if (!tilesetGrid_->generateMipmap()) {
        spdlog::warn("\"{}\": Unable to generate mipmap for texture.", filenameGrid);
    }
    DebugScreen::instance()->registerTexture("tilesetGrid", tilesetGrid_);

    tilesetNoGrid_ = &resource.getTexture(filenameNoGrid, true);
    loadTileset(filenameNoGrid, tilesetNoGrid_, tileWidth);
    tilesetNoGrid_->setSmooth(true);
    if (!tilesetNoGrid_->generateMipmap()) {
        spdlog::warn("\"{}\": Unable to generate mipmap for texture.", filenameNoGrid);
    }
    DebugScreen::instance()->registerTexture("tilesetNoGrid", tilesetNoGrid_);

    ChunkDrawable::setupTextureData(tilesetGrid_->getSize(), tileWidth);
    ChunkRender::setupTextureData(tileWidth);
    Chunk::setupChunks();
}

Board::Board() :    // FIXME we really should be doing member initialization list for all members (needs to be fixed in other classes).
    fileStorage_(new LegacyFileFormat()),
    maxSize_(),
    extraLogicStates_(false),
    notesText_(),
    chunks_(),
    chunkDrawables_(),
    currentLod_(0),
    chunkRenderCache_(),
    lastVisibleArea_(0, 0, 0, 0),
    debugChunkBorder_(sf::Lines),
    debugDrawChunkBorder_(false) {

    for (size_t i = 0; i < chunkRenderCache_.size(); ++i) {
        chunkRenderCache_[i].setLod(i);
    }

    // FIXME this may not be the best way to handle empty chunk, we may want to revert back to keeping an emptyChunk_ member.
    getChunk(ChunkRender::EMPTY_CHUNK_COORDS);
}

void Board::setRenderArea(const OffsetView& offsetView, float zoom) {
    currentLod_ = static_cast<int>(std::max(std::floor(std::log2(zoom)), 0.0f));
    DebugScreen::instance()->getField("lod").setString(fmt::format("Lod: {}", currentLod_));

    // Determine the dimensions of the VertexBuffer we need to draw all of the
    // visible chunks at the max zoom level (for current level-of-detail).
    const sf::Vector2f maxViewSize = offsetView.getSize() / zoom * static_cast<float>(1 << (currentLod_ + 1));
    const int chunkWidthTexels = Chunk::WIDTH * static_cast<int>(tileWidth_);
    const sf::Vector2u maxChunkArea = {
        static_cast<unsigned int>(std::ceil(std::round(maxViewSize.x) / chunkWidthTexels)) + 1,
        static_cast<unsigned int>(std::ceil(std::round(maxViewSize.y) / chunkWidthTexels)) + 1
    };

    ChunkRender& currentChunkRender = chunkRenderCache_[currentLod_];
    currentChunkRender.resize(chunkDrawables_, maxChunkArea);

    sf::Vector2i topLeft = {
        static_cast<int>(std::floor((offsetView.getCenter().x - offsetView.getSize().x / 2.0f) / chunkWidthTexels)),
        static_cast<int>(std::floor((offsetView.getCenter().y - offsetView.getSize().y / 2.0f) / chunkWidthTexels))
    };
    topLeft += offsetView.getCenterOffset();
    sf::Vector2i bottomRight = {
        static_cast<int>(std::floor((offsetView.getCenter().x + offsetView.getSize().x / 2.0f) / chunkWidthTexels)),
        static_cast<int>(std::floor((offsetView.getCenter().y + offsetView.getSize().y / 2.0f) / chunkWidthTexels))
    };
    bottomRight += offsetView.getCenterOffset();
    sf::IntRect visibleArea(topLeft, bottomRight - topLeft + sf::Vector2i(1, 1));

    if (lastVisibleArea_ != visibleArea) {
        spdlog::debug("Visible chunk area changed, now ({}, {}) to ({}, {}).",
            visibleArea.left, visibleArea.top, visibleArea.left + visibleArea.width - 1, visibleArea.top + visibleArea.height - 1
        );
        lastVisibleArea_ = visibleArea;
    }

    updateRender();
    currentChunkRender.updateVisibleArea(chunkDrawables_, lastVisibleArea_);
}

void Board::setMaxSize(const sf::Vector2u& size) {
    maxSize_ = size;
}

void Board::setExtraLogicStates(bool extraLogicStates) {
    extraLogicStates_ = extraLogicStates;
}

void Board::setNotesString(const sf::String& notes) {
    notesText_.setString(notes);
}

const sf::Vector2u& Board::getMaxSize() const {
    return maxSize_;
}

bool Board::getExtraLogicStates() const {
    return extraLogicStates_;
}

const sf::String& Board::getNotesString() const {
    return notesText_.getString();
}

const std::unordered_map<ChunkCoords::repr, Chunk>& Board::getLoadedChunks() const {
    return chunks_;
}

constexpr int constLog2(int x) {
    return x == 1 ? 0 : 1 + constLog2(x / 2);
}

Tile Board::accessTile(int x, int y) {
    // First method using floor division and positive modulus:
    // chunkCoordinate = static_cast<int>(std::floor(static_cast<double>(x) / Chunk::WIDTH))
    // positionInChunk = (x % Chunk::WIDTH + Chunk::WIDTH) % Chunk::WIDTH

    // Improved method since Chunk::WIDTH is a power of 2:
    constexpr int widthLog2 = constLog2(Chunk::WIDTH);
    auto& chunk = getChunk(ChunkCoords::pack(x >> widthLog2, y >> widthLog2));
    return chunk.accessTile(x & (Chunk::WIDTH - 1), y & (Chunk::WIDTH - 1));
}

void Board::loadFromFile(const fs::path& filename) {
    spdlog::info("Loading file \"{}\".", filename);
    std::ifstream inputFile(filename);
    float version = FileStorage::getFileVersion(filename, inputFile);
    if (version < 0.0) {
        throw std::runtime_error("\"" + filename.string() + "\": unknown file version.");
    }
    while (!fileStorage_->validateFileVersion(version)) {
        spdlog::debug("FileStorage not compatible with current version, trying LegacyFileFormat.");
        fileStorage_.reset(new LegacyFileFormat());
        if (fileStorage_->validateFileVersion(version)) {
            break;
        }
        spdlog::debug("FileStorage not compatible with current version, trying RegionFileFormat.");
        fileStorage_.reset(new RegionFileFormat());
        if (fileStorage_->validateFileVersion(version)) {
            break;
        }
        throw std::runtime_error("\"" + filename.string() + "\": invalid file version " + std::to_string(version) + ".");
    }
    fileStorage_->loadFromFile(*this, filename, inputFile);
}

void Board::saveToFile() {
    spdlog::info("Saving file.");


    fileStorage_.reset(new RegionFileFormat());


    fileStorage_->saveToFile(*this);
}

void Board::debugSetDrawChunkBorder(bool enabled) {
    debugDrawChunkBorder_ = enabled;
}

unsigned int Board::debugGetChunksDrawn() const {
    return lastVisibleArea_.width * lastVisibleArea_.height;
}

Chunk& Board::getChunk(ChunkCoords::repr coords) {
    auto chunk = chunks_.find(coords);
    if (chunk == chunks_.end()) {
        spdlog::debug("Allocating new chunk at ({}, {})", ChunkCoords::x(coords), ChunkCoords::y(coords));

        chunk = chunks_.emplace(std::piecewise_construct, std::forward_as_tuple(coords), std::tuple<>()).first;
        chunkDrawables_[coords].setChunk(&chunk->second);
    }
    return chunk->second;
}

void Board::pruneChunkDrawables() {
    spdlog::debug("Pruning chunkDrawables, size is {}.", chunkDrawables_.size());
    auto newLast = std::remove_if(chunkDrawables_.begin(), chunkDrawables_.end(), [](const decltype(chunkDrawables_)::value_type& chunkDrawable) {
        if (!chunkDrawable.second.hasAnyRenderIndex() && chunkDrawable.second.getChunk() == nullptr) {
            spdlog::debug("Pruning ChunkDrawable at ({}, {})", ChunkCoords::x(chunkDrawable.first), ChunkCoords::y(chunkDrawable.first));
            return true;
        } else {
            return false;
        }
    });
    chunkDrawables_.erase(newLast, chunkDrawables_.end());
}

void Board::updateRender() {
    auto drawChunk = [this](const ChunkDrawable& chunkDrawable, bool& allocatedBlock, ChunkCoords::repr coords) {
        if (chunkDrawable.getRenderIndex(currentLod_) == -1) {
            chunkRenderCache_[currentLod_].allocateBlock(chunkDrawables_, coords, lastVisibleArea_);
            allocatedBlock = true;
        }
        sf::RenderStates states;
        states.texture = tilesetGrid_;
        chunkRenderCache_[currentLod_].drawChunk(chunkDrawable, states);
    };

    bool emptyChunkVisible = false, allocatedBlock = false;
    for (int y = lastVisibleArea_.top; y < lastVisibleArea_.top + lastVisibleArea_.height; ++y) {
        // Optimize lookup of the chunkDrawable in the loop by finding first one
        // at or after the current position, then just increment the iterator
        // within the loop. This works since the x-coordinate (as unsigned value
        // in ChunkCoords) is in the lower significant bits, and the ChunkCoords
        // value is always increasing in the loop.
        auto chunkDrawable = chunkDrawables_.upper_bound(ChunkCoords::pack(lastVisibleArea_.left - 1, y));
        for (int x = lastVisibleArea_.left; x < lastVisibleArea_.left + lastVisibleArea_.width; ++x) {
            if (chunkDrawable == chunkDrawables_.end() || chunkDrawable->first != ChunkCoords::pack(x, y)) {
                emptyChunkVisible = true;
            } else {
                if (chunkDrawable->second.isRenderDirty(currentLod_)) {
                    drawChunk(chunkDrawable->second, allocatedBlock, ChunkCoords::pack(x, y));
                }
                ++chunkDrawable;
            }
        }
    }
    const auto& emptyChunk = chunkDrawables_.at(ChunkRender::EMPTY_CHUNK_COORDS);
    if (emptyChunkVisible && emptyChunk.isRenderDirty(currentLod_)) {
        drawChunk(emptyChunk, allocatedBlock, ChunkRender::EMPTY_CHUNK_COORDS);
    }
    if (allocatedBlock) {
        pruneChunkDrawables();
    }
    chunkRenderCache_[currentLod_].display();
}

void Board::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(chunkRenderCache_[currentLod_], states);

    DebugScreen::instance()->profilerEvent("Board::draw draw_debug");
    if (debugDrawChunkBorder_) {
        debugChunkBorder_.resize(lastVisibleArea_.width * lastVisibleArea_.height * 4);
        const int chunkWidthTexels = Chunk::WIDTH * static_cast<int>(tileWidth_);
        unsigned int i = 0;
        for (int y = 0; y < lastVisibleArea_.height; ++y) {
            auto chunkDrawable = chunkDrawables_.upper_bound(ChunkCoords::pack(lastVisibleArea_.left - 1, lastVisibleArea_.top + y));
            float yChunkPos = static_cast<float>(y * chunkWidthTexels);
            for (int x = 0; x < lastVisibleArea_.width; ++x) {
                float xChunkPos = static_cast<float>(x * chunkWidthTexels);
                sf::Color borderColor = sf::Color::Blue;
                if (chunkDrawable != chunkDrawables_.end() && chunkDrawable->first == ChunkCoords::pack(lastVisibleArea_.left + x, lastVisibleArea_.top + y)) {
                    if (chunkDrawable->second.getChunk() != nullptr) {
                        borderColor = sf::Color::Yellow;
                    }
                    ++chunkDrawable;
                }

                debugChunkBorder_[i + 0].position = {xChunkPos, yChunkPos};
                debugChunkBorder_[i + 0].color = borderColor;
                debugChunkBorder_[i + 1].position = {xChunkPos + chunkWidthTexels, yChunkPos};
                debugChunkBorder_[i + 1].color = borderColor;
                debugChunkBorder_[i + 2].position = {xChunkPos, yChunkPos};
                debugChunkBorder_[i + 2].color = borderColor;
                debugChunkBorder_[i + 3].position = {xChunkPos, yChunkPos + chunkWidthTexels};
                debugChunkBorder_[i + 3].color = borderColor;
                i += 4;
            }
        }
        target.draw(debugChunkBorder_, states);
    }
    DebugScreen::instance()->profilerEvent("Board::draw draw_debug_done");
}
