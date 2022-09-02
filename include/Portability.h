#pragma

// Debug
namespace tinylog
{
    #ifdef NDEBUG
    constexpr auto kIsDebug = false;
    #else
    constexpr auto kIsDebug = true; 
    #endif
} // namespace tinylog
