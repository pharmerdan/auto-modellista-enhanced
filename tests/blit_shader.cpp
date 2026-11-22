#include <webgpu/webgpu.h>
#include <webgpu/wgpu.h>

#include <cstdio>
#include <cstring>

extern const char kBlitShader[];

namespace {

WGPUInstance instance = nullptr;
WGPUAdapter adapter = nullptr;
WGPUDevice device = nullptr;
int errors = 0;

WGPUStringView view(const char *text) { return WGPUStringView{text, WGPU_STRLEN}; }

void on_error(WGPUDevice const *, WGPUErrorType, WGPUStringView message, void *, void *) {
    ++errors;
    std::fprintf(stderr, "WebGPU validation error: %.*s\n", static_cast<int>(message.length == WGPU_STRLEN ? std::strlen(message.data) : message.length), message.data);
}

bool open_device() {
    WGPUInstanceExtras extras = {};
    extras.chain.sType = static_cast<WGPUSType>(WGPUSType_InstanceExtras);
    extras.backends = WGPUInstanceBackend_Primary;
    WGPUInstanceDescriptor id = WGPU_INSTANCE_DESCRIPTOR_INIT;
    id.nextInChain = &extras.chain;
    instance = wgpuCreateInstance(&id);
    if (!instance) return false;

    bool done = false;
    WGPURequestAdapterOptions options = WGPU_REQUEST_ADAPTER_OPTIONS_INIT;
    options.powerPreference = WGPUPowerPreference_HighPerformance;
    WGPURequestAdapterCallbackInfo acb = WGPU_REQUEST_ADAPTER_CALLBACK_INFO_INIT;
    acb.mode = WGPUCallbackMode_AllowSpontaneous;
    acb.callback = [](WGPURequestAdapterStatus status, WGPUAdapter found, WGPUStringView message, void *out, void *flag) {
        if (status == WGPURequestAdapterStatus_Success) *static_cast<WGPUAdapter *>(out) = found;
        else std::fprintf(stderr, "Adapter request failed: %.*s\n", static_cast<int>(message.length), message.data);
        *static_cast<bool *>(flag) = true;
    };
    acb.userdata1 = &adapter;
    acb.userdata2 = &done;
    wgpuInstanceRequestAdapter(instance, &options, acb);
    while (!done) wgpuInstanceProcessEvents(instance);
    if (!adapter) return false;

    WGPUDeviceDescriptor dd = WGPU_DEVICE_DESCRIPTOR_INIT;
    dd.uncapturedErrorCallbackInfo.callback = on_error;
    done = false;
    WGPURequestDeviceCallbackInfo dcb = WGPU_REQUEST_DEVICE_CALLBACK_INFO_INIT;
    dcb.mode = WGPUCallbackMode_AllowSpontaneous;
    dcb.callback = [](WGPURequestDeviceStatus status, WGPUDevice found, WGPUStringView message, void *out, void *flag) {
        if (status == WGPURequestDeviceStatus_Success) *static_cast<WGPUDevice *>(out) = found;
        else std::fprintf(stderr, "Device request failed: %.*s\n", static_cast<int>(message.length), message.data);
        *static_cast<bool *>(flag) = true;
    };
    dcb.userdata1 = &device;
    dcb.userdata2 = &done;
    wgpuAdapterRequestDevice(adapter, &dd, dcb);
    while (!done) wgpuInstanceProcessEvents(instance);
    return device != nullptr;
}

} // namespace

int main() {
    if (!open_device()) {
        std::puts("SKIP no WebGPU adapter is exposed in this environment");
        return 77;
    }

    WGPUShaderSourceWGSL source = WGPU_SHADER_SOURCE_WGSL_INIT;
    source.code = view(kBlitShader);
    WGPUShaderModuleDescriptor md = WGPU_SHADER_MODULE_DESCRIPTOR_INIT;
    md.nextInChain = &source.chain;
    WGPUShaderModule module = wgpuDeviceCreateShaderModule(device, &md);

    WGPUBindGroupLayoutEntry entries[2] = {WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT, WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT};
    entries[0].binding = 0;
    entries[0].visibility = WGPUShaderStage_Fragment;
    entries[0].buffer.type = WGPUBufferBindingType_ReadOnlyStorage;
    entries[1].binding = 1;
    entries[1].visibility = WGPUShaderStage_Fragment;
    entries[1].buffer.type = WGPUBufferBindingType_Uniform;
    WGPUBindGroupLayoutDescriptor ld = WGPU_BIND_GROUP_LAYOUT_DESCRIPTOR_INIT;
    ld.entryCount = 2;
    ld.entries = entries;
    WGPUBindGroupLayout layout = wgpuDeviceCreateBindGroupLayout(device, &ld);
    WGPUPipelineLayoutDescriptor pld = WGPU_PIPELINE_LAYOUT_DESCRIPTOR_INIT;
    pld.bindGroupLayoutCount = 1;
    pld.bindGroupLayouts = &layout;
    WGPUPipelineLayout pipeline_layout = wgpuDeviceCreatePipelineLayout(device, &pld);

    WGPUColorTargetState color = WGPU_COLOR_TARGET_STATE_INIT;
    color.format = WGPUTextureFormat_BGRA8Unorm;
    WGPUFragmentState fragment = WGPU_FRAGMENT_STATE_INIT;
    fragment.module = module;
    fragment.entryPoint = view("fs");
    fragment.targetCount = 1;
    fragment.targets = &color;
    WGPURenderPipelineDescriptor rd = WGPU_RENDER_PIPELINE_DESCRIPTOR_INIT;
    rd.layout = pipeline_layout;
    rd.vertex.module = module;
    rd.vertex.entryPoint = view("vs");
    rd.fragment = &fragment;
    WGPURenderPipeline pipeline = wgpuDeviceCreateRenderPipeline(device, &rd);
    wgpuDevicePoll(device, true, nullptr);

    const bool ok = pipeline != nullptr && errors == 0;
    std::printf("%s presentation WGSL compiles and creates a render pipeline\n", ok ? "PASS" : "FAIL");
    if (pipeline) wgpuRenderPipelineRelease(pipeline);
    wgpuPipelineLayoutRelease(pipeline_layout);
    wgpuBindGroupLayoutRelease(layout);
    wgpuShaderModuleRelease(module);
    wgpuDeviceRelease(device);
    wgpuAdapterRelease(adapter);
    wgpuInstanceRelease(instance);
    return ok ? 0 : 1;
}
