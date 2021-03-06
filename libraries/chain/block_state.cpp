#include <eosio/chain/block_state.hpp>
#include <eosio/chain/exceptions.hpp>

namespace eosio { namespace chain {

   namespace {
      constexpr auto additional_sigs_eid = additional_block_signatures_extension::extension_id();

      /**
       * Given a complete signed block, extract the validated additional signatures if present;
       *
       * @param b complete signed block
       * @return the list of additional signatures
       * @throws if additional signatures are present before being supported by protocol feature activations
       */
      vector<signature_type> extract_additional_signatures( const signed_block_ptr& b )
      {
         auto exts = b->validate_and_extract_extensions();

         if ( exts.count(additional_sigs_eid) > 0 ) {
            auto& additional_sigs = exts.lower_bound(additional_sigs_eid)->second.get<additional_block_signatures_extension>();

            return std::move(additional_sigs.signatures);
         }

         return {};
      }

      /**
       * Given a pending block header state, wrap the promotion to a block header state such that additional signatures
       * can be allowed based on activations *prior* to the promoted block and properly injected into the signed block
       * that is previously constructed and mutated by the promotion
       *
       * This cleans up lifetime issues involved with accessing activated protocol features and moving from the
       * pending block header state
       *
       * @param cur the pending block header state to promote
       * @param b the signed block that will receive signatures during this process
       * @param pfs protocol feature set for digest access
       * @param extras all the remaining parameters that pass through
       * @return the block header state
       * @throws if the block was signed with multiple signatures before the extension is allowed
       */

      template<typename ...Extras>
      block_header_state inject_additional_signatures( pending_block_header_state&& cur,
                                                       signed_block& b,
                                                       Extras&& ... extras )
      {
         block_header_state result = std::move(cur).finish_next(b, std::forward<Extras>(extras)...); //签名

         if (!result.additional_signatures.empty()) {
            // as an optimization we don't copy this out into the legitimate extension structure as it serializes
            // the same way as the vector of signatures
            static_assert(fc::reflector<additional_block_signatures_extension>::total_member_count == 1);
            static_assert(std::is_same_v<decltype(additional_block_signatures_extension::signatures), std::vector<signature_type>>);

            emplace_extension(b.block_extensions, additional_sigs_eid, fc::raw::pack( result.additional_signatures ));
         }

         return result;
      }

   }

   block_state::block_state( const block_header_state& prev,
                             signed_block_ptr b,
                             bool skip_validate_signee
                           )
   :block_header_state( prev.next( *b, extract_additional_signatures(b), skip_validate_signee ) )
   ,block( std::move(b) )
   {}

   block_state::block_state( pending_block_header_state&& cur,
                             signed_block_ptr&& b,
                             vector<transaction_metadata_ptr>&& trx_metas,
                             const signer_callback_type& signer
                           )
   :block_header_state( inject_additional_signatures( std::move(cur), *b, signer ) )
   ,block( std::move(b) )
   ,_pub_keys_recovered( true ) // called by produce_block so signature recovery of trxs must have been done
   ,_cached_trxs( std::move(trx_metas) )
   {}

} } /// eosio::chain
