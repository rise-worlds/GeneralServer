#pragma once
#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <vector>

//wabt includes
#include <src/binary-reader.h>
#include <src/common.h>
#include <src/cast.h>
#include <src/interp/interp.h>
#include <src/interp/binary-reader-interp.h>
#include <src/error-formatter.h>
#include <src/make-unique.h>

using namespace wabt;
using namespace wabt::interp;

namespace example {
    namespace wabt {
        class wabt_test {
        public:
            void Init() {
                HostModule* host_module = m_env.AppendHostModule("host");
                m_executor = MakeUnique<Executor>(&m_env);
                std::pair<Memory*, Index> pair = host_module->AppendMemoryExport("mem", Limits(1));
                
                using namespace std::placeholders;
                
                host_module->AppendFuncExport(
                    "fill_buf", {{Type::I32, Type::I32}, {Type::I32}},
                    std::bind(&wabt_test::FillBufCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
                host_module->AppendFuncExport(
                    "buf_done", {{Type::I32, Type::I32}, {}},
                    std::bind(&wabt_test::BufDoneCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
                m_memory = pair.first;
            }
            ~wabt_test() {
                m_executor.reset();
            }

            ::wabt::Result LoadModule(const std::vector<uint8_t>& data) {
                Errors errors;
                ReadBinaryOptions options;
                return ReadBinaryInterp(&m_env, data.data(), data.size(), options, &errors,
                                        &m_module);
            }

            interp::ExecResult Run() {
                return m_executor->RunExportByName(m_module, "rot13", {});
            }

            std::string string_data;

            interp::Result FillBufCallback(const interp::HostFunc* func,
                                            const interp::FuncSignature* sig,
                                            const interp::TypedValues& args,
                                            interp::TypedValues& results) {

                uint32_t ptr = args[0].get_i32();
                uint32_t max_size = args[1].get_i32();
                uint32_t size = std::min(max_size, uint32_t(string_data.size()));

                std::copy(string_data.begin(), string_data.begin() + size,
                        m_memory->data.begin() + ptr);

                results[0].set_i32(size);
                return interp::Result::Ok;
            }

            interp::Result BufDoneCallback(const interp::HostFunc* func,
                                            const interp::FuncSignature* sig,
                                            const interp::TypedValues& args,
                                            interp::TypedValues& results) {

                uint32_t ptr = args[0].get_i32();
                uint32_t size = args[1].get_i32();

                string_data.resize(size);
                std::copy(m_memory->data.begin() + ptr, m_memory->data.begin() + ptr + size,
                        string_data.begin());

                return interp::Result::Ok;
            }
        private:
            interp::Environment m_env;
            interp::Memory* m_memory;
            interp::DefinedModule* m_module;
            std::unique_ptr<interp::Executor> m_executor;
        };
    }
}