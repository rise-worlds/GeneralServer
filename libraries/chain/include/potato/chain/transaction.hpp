#pragma once
#include <potato/chain/types.hpp>
#include <numeric>

namespace potato::chain
{
    struct action;

    struct transaction
    {
        time_point_sec         expiration;   ///< the time at which a transaction expires
        uint16_t               ref_block_num       = 0U; ///< specifies a block num in the last 2^16 blocks.
        uint32_t               ref_block_prefix    = 0UL; ///< specifies the lower 32 bits of the blockid at get_ref_blocknum
        fc::unsigned_int       max_net_usage_words = 0UL; /// upper limit on total network bandwidth (in 8 byte words) billed for this transaction
        uint8_t                max_cpu_usage_ms    = 0; /// upper limit on the total CPU time billed for this transaction
        fc::unsigned_int       delay_sec           = 0UL; /// number of seconds to delay this transaction for during which it may be canceled.

        vector<action>         context_free_actions;
        vector<action>         actions;

        /**
         * @return the absolute block number given the relative ref_block_num
         */
        block_num_type get_ref_blocknum( block_num_type head_blocknum )const {
            return ((head_blocknum/0xffff)*0xffff) + head_blocknum%0xffff;
        }
        void set_reference_block( const block_id_type& reference_block );
        bool verify_reference_block( const block_id_type& reference_block )const;
        void validate()const;

        transaction_id_type        id()const;
        digest_type                sig_digest( const chain_id_type& chain_id, const vector<bytes>& cfd = vector<bytes>() )const;
        fc::microseconds           get_signature_keys( const vector<signature_type>& signatures,
                                                        const chain_id_type& chain_id,
                                                        fc::time_point deadline,
                                                        const vector<bytes>& cfd,
                                                        flat_set<public_key_type>& recovered_pub_keys,
                                                        bool allow_duplicate_keys = false) const;

        uint32_t total_actions()const { return context_free_actions.size() + actions.size(); }

        account_name first_authorizer()const {
            for( const auto& a : actions ) {
                for( const auto& u : a.authorization )
                return u.actor;
            }
            return account_name();
        }
    };

    struct signed_transaction : public transaction
    {
        signed_transaction() = default;
//      signed_transaction( const signed_transaction& ) = default;
//      signed_transaction( signed_transaction&& ) = default;
        signed_transaction( transaction&& trx, const vector<signature_type>& signatures, const vector<bytes>& context_free_data)
        : transaction(std::move(trx))
        , signatures(signatures)
        , context_free_data(context_free_data)
        {}
        signed_transaction( transaction&& trx, const vector<signature_type>& signatures, vector<bytes>&& context_free_data)
        : transaction(std::move(trx))
        , signatures(signatures)
        , context_free_data(std::move(context_free_data))
        {}

        vector<signature_type>    signatures;
        vector<bytes>             context_free_data; ///< for each context-free action, there is an entry here

        const signature_type&     sign(const private_key_type& key, const chain_id_type& chain_id);
        signature_type            sign(const private_key_type& key, const chain_id_type& chain_id)const;
        fc::microseconds          get_signature_keys( const chain_id_type& chain_id, fc::time_point deadline,
                                                        flat_set<public_key_type>& recovered_pub_keys,
                                                        bool allow_duplicate_keys = false )const;
    };

    struct packed_transaction : fc::reflect_init
    {
        enum class compression_type {
            none = 0,
            zlib = 1,
        };

        packed_transaction() = default;
        packed_transaction(packed_transaction&&) = default;
        explicit packed_transaction(const packed_transaction&) = default;
        packed_transaction& operator=(const packed_transaction&) = delete;
        packed_transaction& operator=(packed_transaction&&) = default;

        explicit packed_transaction(const signed_transaction& t, compression_type _compression = compression_type::none)
        :signatures(t.signatures), compression(_compression), unpacked_trx(t), trx_id(unpacked_trx.id())
        {
            local_pack_transaction();
            local_pack_context_free_data();
        }

        explicit packed_transaction(signed_transaction&& t, compression_type _compression = compression_type::none)
        :signatures(t.signatures), compression(_compression), unpacked_trx(std::move(t)), trx_id(unpacked_trx.id())
        {
            local_pack_transaction();
            local_pack_context_free_data();
        }

        // used by abi_serializer
        packed_transaction( bytes&& packed_txn, vector<signature_type>&& sigs, bytes&& packed_cfd, compression_type _compression );
        packed_transaction( bytes&& packed_txn, vector<signature_type>&& sigs, vector<bytes>&& cfd, compression_type _compression );
        packed_transaction( transaction&& t, vector<signature_type>&& sigs, bytes&& packed_cfd, compression_type _compression );

        uint32_t get_unprunable_size()const;
        uint32_t get_prunable_size()const;

        digest_type packed_digest()const;

        const transaction_id_type& id()const { return trx_id; }
        bytes               get_raw_transaction()const;

        time_point_sec                expiration()const { return unpacked_trx.expiration; }
        const vector<bytes>&          get_context_free_data()const { return unpacked_trx.context_free_data; }
        const transaction&            get_transaction()const { return unpacked_trx; }
        const signed_transaction&     get_signed_transaction()const { return unpacked_trx; }
        const vector<signature_type>& get_signatures()const { return signatures; }
        const fc::enum_type<uint8_t,compression_type>& get_compression()const { return compression; }
        const bytes&                  get_packed_context_free_data()const { return packed_context_free_data; }
        const bytes&                  get_packed_transaction()const { return packed_trx; }

    private:
        void local_unpack_transaction(vector<bytes>&& context_free_data);
        void local_unpack_context_free_data();
        void local_pack_transaction();
        void local_pack_context_free_data();

        friend struct fc::reflector<packed_transaction>;
        friend struct fc::reflector_init_visitor<packed_transaction>;
        friend struct fc::has_reflector_init<packed_transaction>;
        void reflector_init();
    private:
        vector<signature_type>                  signatures;
        fc::enum_type<uint8_t,compression_type> compression;
        bytes                                   packed_context_free_data;
        bytes                                   packed_trx;

    private:
        // cache unpacked trx, for thread safety do not modify after construction
        signed_transaction                      unpacked_trx;
        transaction_id_type                     trx_id;
    };
}

FC_REFLECT(potato::chain::transaction, (expiration)(ref_block_num)(ref_block_prefix)
                                        (max_net_usage_words)(max_cpu_usage_ms)(delay_sec)
                                        (context_free_actions)(actions))
FC_REFLECT_DERIVED(potato::chain::signed_transaction, (potato::chain::transaction), (signatures)(context_free_data) )
FC_REFLECT_ENUM(potato::chain::packed_transaction::compression_type, (none)(zlib))
// @ignore unpacked_trx
FC_REFLECT(potato::chain::packed_transaction, (signatures)(compression)(packed_context_free_data)(packed_trx))
