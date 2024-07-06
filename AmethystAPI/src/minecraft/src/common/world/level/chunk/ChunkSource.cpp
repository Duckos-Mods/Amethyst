#include <minecraft/src/common/world/level/chunk/ChunkSource.hpp>
#include <amethyst/Memory.hpp>
#include <amethyst/MinecraftVtables.hpp>


ChunkSource::ChunkSource(Dimension* dimension, int side) : Bedrock::EnableNonOwnerReferences()
{
    mChunkSide = side;
    Level* Level = nullptr;
    if (dimension)
        mLevel = dimension->mLevel;
    mLevel = Level;
    mDimension = dimension;
    mParent = nullptr;

    // What the hell man
    void** vtable = *(void***)this;
    // 48 8D 05 ? ? ? ? 48 89 07 C7 47 ? ? ? ? ? 33 ED
    static void* ChunkSourceVFtable = nullptr;
    if (!ChunkSourceVFtable) InitializeVtbl(ChunkSourceVFtable, "48 8D 05 ? ? ? ? 48 89 07 C7 47 ? ? ? ? ? 33 ED");

}