// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/FileMapping.hpp"
#include <gsl/gsl>

namespace TTauri {

struct FileView {
    std::shared_ptr<FileMapping> fileMappingObject;
    std::shared_ptr<gsl::span<std::byte>> _bytes;
    size_t offset;

    FileView(std::shared_ptr<FileMapping> const& mappingObject, size_t offset, size_t size);
    FileView(URL const &location, AccessMode accessMode=AccessMode::RDONLY, size_t offset=0, size_t size=0);
    ~FileView() = default;

    FileView() = delete;
    FileView(FileView const &other) noexcept;
    FileView(FileView &&other) noexcept;
    FileView &operator=(FileView const &other) noexcept;
    FileView &operator=(FileView &&other) noexcept;

    AccessMode accessMode() const noexcept { return fileMappingObject->accessMode(); }
    URL const &location() const noexcept { return fileMappingObject->location(); }

    gsl::span<std::byte> bytes() noexcept { return *_bytes; }
    gsl::span<std::byte const> bytes() const noexcept { return *_bytes; }

    void flush(void* base, size_t size);

    static void unmap(gsl::span<std::byte> *bytes) noexcept;

    static std::shared_ptr<FileMapping> findOrCreateFileMappingObject(URL const& path, AccessMode accessMode, size_t size);
};

}
