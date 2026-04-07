#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>

namespace zen {
namespace blockchain {

// 区块链基础

// 哈希类型
using hash256 = std::array<uint8_t, 32>;
using hash160 = std::array<uint8_t, 20>;

// 交易
class transaction {
public:
    transaction();
    transaction(const std::string& from, const std::string& to, double amount);
    
    // 交易信息
    std::string get_from() const { return from_; }
    std::string get_to() const { return to_; }
    double get_amount() const { return amount_; }
    int64_t get_timestamp() const { return timestamp_; }
    std::string get_id() const { return id_; }
    
    // 计算哈希
    hash256 compute_hash() const;
    
    // 签名
    void sign(const crypto::ecc::key_pair& key_pair);
    bool verify_signature(const std::string& public_key) const;
    
    // 序列化
    std::vector<uint8_t> serialize() const;
    static transaction deserialize(const std::vector<uint8_t>& data);
    
private:
    std::string from_;
    std::string to_;
    double amount_;
    int64_t timestamp_;
    std::string id_;
    std::vector<uint8_t> signature_;
};

// 区块
class block {
public:
    block();
    block(int index, const std::vector<transaction>& transactions,
            const hash256& previous_hash, std::string miner = "");
    
    // 区块信息
    int get_index() const { return index_; }
    hash256 get_hash() const { return hash_; }
    hash256 get_previous_hash() const { return previous_hash_; }
    int64_t get_timestamp() const { return timestamp_; }
    double get_nonce() const { return nonce_; }
    int get_difficulty() const { return difficulty_; }
    std::string get_miner() const { return miner_; }
    
    const std::vector<transaction>& get_transactions() const { return transactions_; }
    
    // 计算哈希
    hash256 compute_hash() const;
    
    // 挖矿（工作量证明）
    bool mine(int difficulty);
    
    // 验证
    bool is_valid() const;
    
    // 序列化
    std::vector<uint8_t> serialize() const;
    static block deserialize(const std::vector<uint8_t>& data);
    
private:
    int index_;
    hash256 hash_;
    hash256 previous_hash_;
    int64_t timestamp_;
    std::vector<transaction> transactions_;
    double nonce_;
    int difficulty_;
    std::string miner_;
    
    void compute_hash();
};

// 区块链
class blockchain {
public:
    blockchain();
    ~blockchain();
    
    // 添加区块
    bool add_block(block block);
    bool add_transaction(const transaction& tx);
    
    // 挖矿新区块
    block mine_block(const std::string& miner, int difficulty = 4);
    
    // 验证
    bool is_valid() const;
    bool is_block_valid(const block& block) const;
    
    // 查询
    block get_block(int index) const;
    block get_block(const hash256& hash) const;
    std::vector<transaction> get_transactions(const std::string& address) const;
    double get_balance(const std::string& address) const;
    
    // 链信息
    int get_height() const { return chain_.size(); }
    block get_latest_block() const { return chain_.back(); }
    size_t get_difficulty() const { return difficulty_; }
    
    // 待确认交易
    const std::vector<transaction>& get_pending_transactions() const {
        return pending_transactions_;
    }
    
    // 序列化/反序列化
    std::vector<uint8_t> serialize() const;
    bool deserialize(const std::vector<uint8_t>& data);
    bool save(const std::string& path) const;
    bool load(const std::string& path);
    
private:
    // 创世区块
    void create_genesis_block();
    
    // 验证交易
    bool is_transaction_valid(const transaction& tx) const;
    
    std::vector<block> chain_;
    std::vector<transaction> pending_transactions_;
    size_t difficulty_;
    
    threading::mutex chain_mutex_;
};

// 钱包
class wallet {
public:
    wallet();
    explicit wallet(const std::string& private_key);
    
    // 生成密钥
    void generate_key();
    void generate_key(const std::string& seed);
    
    // 获取地址
    std::string get_address() const;
    std::string get_public_key() const;
    std::string get_private_key() const;
    
    // 签名
    transaction sign_transaction(const transaction& tx);
    
    // 密钥对
    const crypto::ecc::key_pair& get_key_pair() const { return key_pair_; }
    
    // 序列化
    std::string serialize() const;
    static wallet deserialize(const std::string& data);
    
    // 保存/加载
    bool save(const std::string& path) const;
    bool load(const std::string& path);
    
private:
    crypto::ecc::key_pair key_pair_;
    std::string address_;
    
    void derive_address();
};

// 共识算法
enum class consensus_type {
    pow,      // 工作量证明
    pos,      // 权益证明
    dpos,     // 委托权益证明
    pbft      // 实用拜占庭容错
};

// 共识接口
class consensus {
public:
    virtual ~consensus() = default;
    virtual bool validate_block(const block& block) = 0;
    virtual bool select_leader() = 0;
};

// 工作量证明共识
class proof_of_work : public consensus {
public:
    proof_of_work(int difficulty = 4);
    
    bool validate_block(const block& block) override;
    bool select_leader() override;
    
    void set_difficulty(int difficulty) { difficulty_ = difficulty; }
    int get_difficulty() const { return difficulty_; }
    
private:
    int difficulty_;
};

// 智能合约（简化版）
class smart_contract {
public:
    using state = std::unordered_map<std::string, std::string>;
    using contract_code = std::function<void(const transaction&, state&)>;
    
    smart_contract(const std::string& name, contract_code code);
    
    void execute(const transaction& tx);
    
    const std::string& get_name() const { return name_; }
    const state& get_state() const { return state_; }
    
private:
    std::string name_;
    contract_code code_;
    state state_;
    
    mutable threading::mutex state_mutex_;
};

} // namespace blockchain
} // namespace zen
