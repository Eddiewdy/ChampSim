#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <deque>
#include <memory>
#include <numeric>
#include <string>
#include <cassert>
#include <unordered_map>
#include <unordered_set>
#include <chrono>

using namespace std;

constexpr std::size_t NUM_INSTR_DESTINATIONS_SPARC = 4;
constexpr std::size_t NUM_INSTR_DESTINATIONS = 2;
constexpr std::size_t NUM_INSTR_SOURCES = 4;

struct range_size_input_instr {
    unsigned long long ip;
    unsigned char is_branch;
    unsigned char branch_taken;
    unsigned char destination_registers[NUM_INSTR_DESTINATIONS];
    unsigned char source_registers[NUM_INSTR_SOURCES];
    unsigned long long destination_memory[NUM_INSTR_DESTINATIONS];
    unsigned long long source_memory[NUM_INSTR_SOURCES];
    unsigned long long destination_range_id[NUM_INSTR_DESTINATIONS];
    unsigned long long source_range_id[NUM_INSTR_SOURCES];
    unsigned long long destination_size[NUM_INSTR_DESTINATIONS];
    unsigned long long source_size[NUM_INSTR_SOURCES];
};

void print_instruction(const range_size_input_instr& curr_instr) {
    std::cout << "[Instruction Write] IP: " << std::hex << curr_instr.ip << std::dec << std::endl;

    for (std::size_t i = 0; i < NUM_INSTR_SOURCES; ++i) {
        if (curr_instr.source_memory[i] != 0) {
            std::cout << "  Source Memory[" << i << "] Addr: " << std::hex << curr_instr.source_memory[i] 
                      << std::dec << " ID: " << curr_instr.source_range_id[i] 
                      << " Size: " << curr_instr.source_size[i] << std::dec << std::endl;
        }
    }

    for (std::size_t i = 0; i < NUM_INSTR_DESTINATIONS; ++i) {
        if (curr_instr.destination_memory[i] != 0) {
            std::cout << "  Destination Memory[" << i << "] Addr: " << std::hex << curr_instr.destination_memory[i] 
                      << std::dec << " ID: " << curr_instr.destination_range_id[i] 
                      << " Size: " << curr_instr.destination_size[i] << std::dec << std::endl;
        }
    }
}

// void calculate_and_print_memory_steps(const std::unordered_map<unsigned long long, std::vector<unsigned long long>>& memory_history, 
//     const std::unordered_map<unsigned long long, std::unordered_set<unsigned long long>>& range_ip_map) {
//     constexpr unsigned long long CACHELINE_SIZE = 64;
//     for (const auto& [range_id, addresses] : memory_history) {
//         if (addresses.size() < 2) continue; // 没有足够数据计算步长

//         std::cout << "Range ID: " << range_id << std::endl;
//         std::cout << "Addresses: ";
//         for (size_t i = 0; i < addresses.size(); ++i) {
//             std::cout << addresses[i] << " ";
//             if ((i + 1) % 10 == 0) { // 每 10 个地址换行
//                 std::cout << std::endl;
//             }
//         }
//         if (addresses.size() % 10 != 0) { // 如果最后一行不满 10 个地址
//             std::cout << std::endl;
//         }

//         // // 计算步长
//         // std::cout << "Steps: ";
//         // for (size_t i = 1; i < addresses.size(); ++i) {
//         //     long long step = static_cast<long long>(addresses[i]) - static_cast<long long>(addresses[i - 1]);
//         //     std::cout << step << " ";
//         // }

//         // 计算步长（按 cacheline 计算）
//         std::cout << "Cacheline Strides: ";
//         size_t stride_count = 0;
//         for (size_t i = 1; i < addresses.size(); ++i) {
//             long long prev_cacheline = addresses[i - 1] / CACHELINE_SIZE;
//             long long curr_cacheline = addresses[i] / CACHELINE_SIZE;

//             long long stride = curr_cacheline - prev_cacheline;
//             std::cout << stride << " ";
//             stride_count++;

//             if (stride_count % 10 == 0) { // 每 10 个步长换行
//                 std::cout << std::endl;
//             }
//         }
//         if ((addresses.size() - 1) % 10 != 0) { // 如果最后一行不满 10 个步长
//             std::cout << std::endl;
//         }
        
//         auto ip_it = range_ip_map.find(range_id);
//         if (ip_it != range_ip_map.end()) {
//             std::cout << "Unique IP Count: " << ip_it->second.size() << std::endl;
//         } else {
//             std::cout << "Unique IP Count: 0" << std::endl;
//         }

//         std::cout << "-----------------" << std::endl;
//     }
// }

void calculate_and_print_memory_steps(
    const std::unordered_map<unsigned long long, std::vector<unsigned long long>>& memory_history,
    const std::unordered_map<unsigned long long, std::unordered_set<unsigned long long>>& range_ip_map,
    const std::unordered_map<unsigned long long, std::vector<std::pair<unsigned long long, unsigned long long>>>& memory_ip_history,
    const std::unordered_map<unsigned long long, unsigned long long>& range_size_map) {
    constexpr unsigned long long CACHELINE_SIZE = 64;

    for (const auto& [range_id, addresses] : memory_history) {
        if (addresses.size() < 2) continue; // 没有足够数据计算步长

        // 从 range_size_map 获取对应的 size 总和
        auto size_it = range_size_map.find(range_id);
        unsigned long long total_size = (size_it != range_size_map.end()) ? size_it->second : 0;

        std::cout << "Range ID: " << range_id 
                  << " | Total Size: " << total_size << " bytes" << std::endl;

        // 打印该 range 的所有地址
        // std::cout << "Addresses: " << std::endl;
        // for (size_t i = 0; i < addresses.size(); ++i) {
        //     std::cout << addresses[i] << " ";
        //     if ((i + 1) % 15 == 0) { // 每 10 个地址换行
        //         std::cout << std::endl;
        //     }
        // }
        // if (addresses.size() % 15 != 0) { // 如果最后一行不满 10 个地址
        //     std::cout << std::endl;
        // }

        // 计算非重复 Cacheline 的 Strides
        std::cout << "Cacheline Strides: " << std::endl;
        size_t stride_count = 0;
        std::unordered_set<unsigned long long> seen_cachelines;
        std::vector<unsigned long long> unique_cachelines;

        for (const auto& address : addresses) {
            unsigned long long cacheline = address / CACHELINE_SIZE;
            if (seen_cachelines.find(cacheline) == seen_cachelines.end()) {
                unique_cachelines.push_back(cacheline);
                seen_cachelines.insert(cacheline);
            }
        }

        for (size_t i = 1; i < unique_cachelines.size(); ++i) {
            long long stride = unique_cachelines[i] - unique_cachelines[i - 1];
            std::cout << stride << " ";
            stride_count++;

            if (stride_count % 15 == 0) { // 每 10 个步长换行
                std::cout << std::endl;
            }
        }
        if ((unique_cachelines.size() - 1) % 15 != 0) { // 如果最后一行不满 10 个步长
            std::cout << std::endl;
        }

        // std::cout << "Steps: ";
        // for (size_t i = 1; i < addresses.size(); ++i) {
        //     long long step = static_cast<long long>(addresses[i]) - static_cast<long long>(addresses[i - 1]);
        //     std::cout << step << " ";
        // }

        // 计算步长（按 cacheline 计算）
        // std::cout << "Cacheline Strides: ";
        // size_t stride_count = 0;
        // for (size_t i = 1; i < addresses.size(); ++i) {
        //     long long prev_cacheline = addresses[i - 1] / CACHELINE_SIZE;
        //     long long curr_cacheline = addresses[i] / CACHELINE_SIZE;

        //     long long stride = curr_cacheline - prev_cacheline;
        //     if (stride == 0) continue;
        //     std::cout << stride << " ";
        //     stride_count++;

        //     if (stride_count % 10 == 0) { // 每 10 个步长换行
        //         std::cout << std::endl;
        //     }
        // }
        // if ((addresses.size() - 1) % 10 != 0) { // 如果最后一行不满 10 个步长
        //     std::cout << std::endl;
        // }

        // 打印唯一 IP 数量
        auto ip_it = range_ip_map.find(range_id);
        if (ip_it != range_ip_map.end()) {
            std::cout << "Unique IP Count: " << ip_it->second.size() << std::endl;
            std::cout << "-----------------" << std::endl;
            // 针对每个 IP 分析其地址历史和步长
            auto ip_address_pairs = memory_ip_history.find(range_id);
            if (ip_address_pairs != memory_ip_history.end()) {
                // 分组每个 IP 的访问地址
                std::unordered_map<unsigned long long, std::vector<unsigned long long>> ip_to_addresses;
                for (const auto& [ip, address] : ip_address_pairs->second) {
                    ip_to_addresses[ip].push_back(address);
                }

                // 逐个 IP 分析其 Cacheline Strides
                for (const auto& [ip, ip_addresses] : ip_to_addresses) {
                    std::cout << "IP: " << std::hex << ip << std::dec << ", Load Instruction Count: " << ip_addresses.size() << std::endl;

                    // 打印该 IP 的访问地址
                    // std::cout << "  Addresses: " << std::endl;
                    // for (size_t i = 0; i < ip_addresses.size(); ++i) {
                    //     std::cout << ip_addresses[i] << " ";
                    //     if ((i + 1) % 15 == 0) {
                    //         std::cout << std::endl;
                    //     }
                    // }
                    // if (ip_addresses.size() % 15 != 0) {
                    //     std::cout << std::endl;
                    // }

                    // if (ip_addresses.size() < 2) {
                    //     std::cout << "  No enough data for strides." << std::endl;
                    //     continue;
                    // }

                    // 计算 Cacheline Strides
                    std::cout << "Cacheline Strides: " << std::endl;
                    size_t ip_stride_count = 0;
                    for (size_t i = 1; i < ip_addresses.size(); ++i) {
                        long long prev_cacheline = ip_addresses[i - 1] / CACHELINE_SIZE;
                        long long curr_cacheline = ip_addresses[i] / CACHELINE_SIZE;

                        long long stride = curr_cacheline - prev_cacheline;
                        if (stride == 0) continue;
                        std::cout << stride << " ";
                        ip_stride_count++;

                        if (ip_stride_count % 15 == 0) {
                            std::cout << std::endl;
                        }
                    }
                    if ((ip_addresses.size() - 1) % 10 != 0) {
                        std::cout << std::endl;
                    }
                    // std::unordered_set<unsigned long long> ip_seen_cachelines;
                    // std::vector<unsigned long long> ip_unique_cachelines;

                    // for (const auto& address : ip_addresses) {
                    //     unsigned long long ip_cacheline = address / CACHELINE_SIZE;
                    //     if (ip_seen_cachelines.find(ip_cacheline) == ip_seen_cachelines.end()) {
                    //         ip_unique_cachelines.push_back(ip_cacheline);
                    //         ip_seen_cachelines.insert(ip_cacheline);
                    //     }
                    // }
                    // stride_count = 0;
                    // std::cout << "Cacheline Strides: " << std::endl;
                    // for (size_t i = 1; i < ip_unique_cachelines.size(); ++i) {
                    //     long long stride = ip_unique_cachelines[i] - ip_unique_cachelines[i - 1];
                    //     std::cout << stride << " ";
                    //     stride_count++;

                    //     if (stride_count % 15 == 0) { // 每 10 个步长换行
                    //         std::cout << std::endl;
                    //     }
                    // }
                    // if ((ip_unique_cachelines.size() - 1) % 15 != 0) { // 如果最后一行不满 10 个步长
                    //     std::cout << std::endl;
                    // }
                    std::cout << std::endl;
                }
            }
        } else {
            std::cout << "Unique IP Count: 0" << std::endl;
        }

        std::cout << "-----------------" << std::endl;
        std::cout << "-----------------" << std::endl;
    }
}









bool has_nonzero_memory(const range_size_input_instr& instr) {
    for (std::size_t i = 0; i < NUM_INSTR_SOURCES; ++i) {
        if (instr.source_memory[i] != 0) {
            return true;
        }
    }
    for (std::size_t i = 0; i < NUM_INSTR_DESTINATIONS; ++i) {
        if (instr.destination_memory[i] != 0) {
            return true;
        }
    }
    return false;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_trace_file>" << std::endl;
        return -1;
    }

    const char* input_filename = argv[1];
    FILE* old_trace = fopen(input_filename, "rb");
    if (old_trace == NULL) {
        std::cerr << "Fail to open input file: " << input_filename << std::endl;
        return -1;
    }
    range_size_input_instr instr;
    size_t instr_count = 0;

    std::unordered_map<unsigned long long, std::vector<unsigned long long>> memory_history;
    std::unordered_map<unsigned long long, std::unordered_set<unsigned long long>> range_ip_map;
    std::unordered_map<unsigned long long, std::vector<std::pair<unsigned long long, unsigned long long>>> memory_ip_history;
    std::unordered_map<unsigned long long, unsigned long long> range_size_map; // 新增 map 存储每个 range 的总大小
    std::size_t source_memory_nonzero_count = 0; // 统计 instr.source_memory[i] != 0 的个数
    std::size_t source_memory_and_range_id_nonzero_count = 0; // 统计 instr.source_memory[i] != 0 && instr.source_range_id[i] != 0 的个数

    while (instr_count < 25000000) {
        if (fread(&instr, sizeof(range_size_input_instr), 1, old_trace) == 1) {
            for (std::size_t i = 0; i < NUM_INSTR_SOURCES; ++i) {
                if (instr.source_memory[i] != 0) {
                source_memory_nonzero_count++;
                if (instr.source_range_id[i] != 0) {
                    source_memory_and_range_id_nonzero_count++;
                    memory_history[instr.source_range_id[i]].push_back(instr.source_memory[i]);
                    range_ip_map[instr.source_range_id[i]].insert(instr.ip);
                    memory_ip_history[instr.source_range_id[i]].emplace_back(instr.ip, instr.source_memory[i]);
                    range_size_map[instr.source_range_id[i]] = instr.source_size[i]; // 累加 source_size
                }
            }
            }
            instr_count++;
        } else {
            break;
        }
    }

    calculate_and_print_memory_steps(memory_history, range_ip_map, memory_ip_history, range_size_map);

    fclose(old_trace);

    double source_memory_object_ratio = static_cast<double>(source_memory_and_range_id_nonzero_count) / static_cast<double>(source_memory_nonzero_count) * 100;

    // 输出统计结果和比例
    std::cout << "MemoryLoad_Count: " << source_memory_nonzero_count << " MemoryLoad_In_Object_Count: " << source_memory_and_range_id_nonzero_count
            << " (" << source_memory_object_ratio << "%)" << std::endl;
    return 0;
}