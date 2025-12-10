// archive_init.cpp - Force initialization of archive format handlers

// 通过引用注册函数来强制链接格式处理器
// 在静态链接时，未被引用的对象文件可能被链接器丢弃

// 前置声明来自 RegisterArc.h 的注册函数
extern "C" void RegisterArc(const void* arcInfo) throw();

namespace {

// 这个函数的作用是强制链接器包含所有格式注册代码
// 虽然它永远不会被调用，但它引用了注册函数，防止被优化掉
void ForceInitArchiveFormats() {
    // 通过引用 RegisterArc 来确保符号不被优化
    volatile auto p = &RegisterArc;
    (void)p;

    // 注意：实际的格式注册通过各个 *Register.cpp 文件中的静态初始化完成
    // REGISTER_ARC_IO 宏创建全局对象，在程序启动时自动注册
}

}  // namespace

// 前置声明CRC初始化函数 - 注意 __fastcall 调用约定
#ifdef _WIN32
extern "C" void __fastcall CrcGenerateTable(void);
#else
extern "C" void CrcGenerateTable(void);
#endif

// 导出一个初始化函数供外部调用（虽然目前不需要显式调用）
extern "C" void InitializeArchiveFormats() {
    // 确保CRC表已初始化（这对7z格式很关键）
    static bool crcInitialized = false;
    if (!crcInitialized) {
        CrcGenerateTable();
        crcInitialized = true;
    }

    // 格式已经通过静态初始化注册
    // 这个函数存在是为了确保这个编译单元被链接
}
