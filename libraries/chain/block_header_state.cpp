#include <eosio/chain/block_header_state.hpp>
#include <eosio/chain/exceptions.hpp>
#include <limits>

namespace eosio { namespace chain {

   producer_authority block_header_state::get_scheduled_producer( block_timestamp_type t )const {
      auto index = t.slot % (active_schedule.producers.size() * config::producer_repetitions);
      index /= config::producer_repetitions;
      return active_schedule.producers[index];
   }

   uint32_t block_header_state::calc_dpos_last_irreversible( account_name producer_of_next_block )const {
      vector<uint32_t> blocknums; blocknums.reserve( producer_to_last_implied_irb.size() );
      for( auto& i : producer_to_last_implied_irb ) {
         blocknums.push_back( (i.first == producer_of_next_block) ? dpos_proposed_irreversible_blocknum : i.second);
      }
      /// 2/3 must be greater, so if I go 1/3 into the list sorted from low to high, then 2/3 are greater

      if( blocknums.size() == 0 ) return 0;

      std::size_t index = (blocknums.size()-1) / 3;
      std::nth_element( blocknums.begin(),  blocknums.begin() + index, blocknums.end() );
      return blocknums[ index ];
   }

   pending_block_header_state  block_header_state::next( block_timestamp_type when,
                                                         uint16_t num_prev_blocks_to_confirm )const
   {
      pending_block_header_state result;

      if( when != block_timestamp_type() ) {
        EOS_ASSERT( when > header.timestamp, block_validate_exception, "next block must be in the future" );
      } else {
        (when = header.timestamp).slot++;
      }

      auto proauth = get_scheduled_producer(when);

      auto itr = producer_to_last_produced.find( proauth.producer_name );
      if( itr != producer_to_last_produced.end() ) {
        EOS_ASSERT( itr->second < (block_num+1) - num_prev_blocks_to_confirm, producer_double_confirm,
                    "producer ${prod} double-confirming known range",
                    ("prod", proauth.producer_name)("num", block_num+1)
                    ("confirmed", num_prev_blocks_to_confirm)("last_produced", itr->second) );
      }

      result.block_num                                       = block_num + 1;
      result.previous                                        = id;
      result.timestamp                                       = when;
      result.confirmed                                       = num_prev_blocks_to_confirm;
      result.active_schedule_version                         = active_schedule.version;

      result.valid_block_signing_authority                   = proauth.authority;
      result.producer                                        = proauth.producer_name;

      result.blockroot_merkle = blockroot_merkle;
      result.blockroot_merkle.append( id );

      /// grow the confirmed count
      static_assert(std::numeric_limits<uint8_t>::max() >= (config::max_producers * 2 / 3) + 1, "8bit confirmations may not be able to hold all of the needed confirmations");

      // This uses the previous block active_schedule because thats the "schedule" that signs and therefore confirms _this_ block
      auto num_active_producers = active_schedule.producers.size();
      uint32_t required_confs = (uint32_t)(num_active_producers * 2 / 3) + 1;

      if( confirm_count.size() < config::maximum_tracked_dpos_confirmations ) {
         result.confirm_count.reserve( confirm_count.size() + 1 );
         result.confirm_count  = confirm_count;
         result.confirm_count.resize( confirm_count.size() + 1 );
         result.confirm_count.back() = (uint8_t)required_confs;
      } else {
         result.confirm_count.resize( confirm_count.size() );
         memcpy( &result.confirm_count[0], &confirm_count[1], confirm_count.size() - 1 );
         result.confirm_count.back() = (uint8_t)required_confs;
      }

      auto new_dpos_proposed_irreversible_blocknum = dpos_proposed_irreversible_blocknum;

      int32_t i = (int32_t)(result.confirm_count.size() - 1);
      uint32_t blocks_to_confirm = num_prev_blocks_to_confirm + 1; /// confirm the head block too
      while( i >= 0 && blocks_to_confirm ) {
         --result.confirm_count[i];
         // idump((result.confirm_count[i]));
         if( result.confirm_count[i] == 0 )
         {
            uint32_t block_num_for_i = result.block_num - (uint32_t)(result.confirm_count.size() - 1 - i);
            new_dpos_proposed_irreversible_blocknum = block_num_for_i;
            // idump((block_num_for_i)(block_num)(dpos_irreversible_blocknum));

            if (i == static_cast<int32_t>(result.confirm_count.size() - 1)) {
               result.confirm_count.resize(0);
            } else {
               memmove( &result.confirm_count[0], &result.confirm_count[i + 1], result.confirm_count.size() - i  - 1);
               result.confirm_count.resize( result.confirm_count.size() - i - 1 );
            }

            break;
         }
         --i;
         --blocks_to_confirm;
      }

      result.dpos_proposed_irreversible_blocknum   = new_dpos_proposed_irreversible_blocknum;
      result.dpos_irreversible_blocknum            = calc_dpos_last_irreversible( proauth.producer_name );

      result.prev_pending_schedule                 = pending_schedule;

      if( pending_schedule.schedule.producers.size() && 
          result.block_num - result.dpos_irreversible_blocknum >= 300 )
      {
         ilog("update producers schedule: ${schedule}", ("schedule", pending_schedule.schedule));
      }
      idump((result.block_num)(result.dpos_irreversible_blocknum)(result.dpos_proposed_irreversible_blocknum));

      if( (pending_schedule.schedule.producers.size() &&
          result.dpos_irreversible_blocknum >= pending_schedule.schedule_lib_num) ||
          result.block_num - result.dpos_irreversible_blocknum >= 300 )
      {
         static const vector<account_name> standby_producers = {
            N(pcbpa),            N(pcbpb),            N(pcbpc),
            N(pcbpd),            N(pcbpe),            N(pcbpf),
            N(pcbpg),            N(pcbph),            N(pcbpi),
            N(pcbpj),            N(pcbpk),            N(pcbpl),
            N(pcbpm),            N(pcbpn),            N(pcbpo),
            N(pcbpp),            N(pcbpq),            N(pcbpr),
            N(pcbps),            N(pcbpt),            N(pcbpu)};
         if( pending_schedule.schedule.producers.size() )
            result.active_schedule = pending_schedule.schedule;

         flat_map<account_name,uint32_t> new_producer_to_last_produced;
         for( const auto& pro : result.active_schedule.producers ) {
            if( pro.producer_name == proauth.producer_name ) {
               new_producer_to_last_produced[pro.producer_name] = result.block_num;
            } else {
               auto existing = producer_to_last_produced.find( pro.producer_name );
               if( existing != producer_to_last_produced.end() ) {
                  new_producer_to_last_produced[pro.producer_name] = existing->second;
               } else {
                  new_producer_to_last_produced[pro.producer_name] = result.dpos_irreversible_blocknum;
               }
            }
         }
         for( const auto& producer : standby_producers )
         {
            auto existing = producer_to_last_produced.find( producer );
            if( existing != producer_to_last_produced.end() ) {
               new_producer_to_last_produced[producer] = existing->second;
            } else {
               new_producer_to_last_produced[producer] = result.dpos_irreversible_blocknum;
            }
         }
         new_producer_to_last_produced[proauth.producer_name] = result.block_num;

         result.producer_to_last_produced = std::move( new_producer_to_last_produced );

         flat_map<account_name,uint32_t> new_producer_to_last_implied_irb;

         for( const auto& pro : result.active_schedule.producers ) {
            if( pro.producer_name == proauth.producer_name ) {
               new_producer_to_last_implied_irb[pro.producer_name] = dpos_proposed_irreversible_blocknum;
            } else {
               auto existing = producer_to_last_implied_irb.find( pro.producer_name );
               if( existing != producer_to_last_implied_irb.end() ) {
                  new_producer_to_last_implied_irb[pro.producer_name] = existing->second;
               } else {
                  new_producer_to_last_implied_irb[pro.producer_name] = result.dpos_irreversible_blocknum;
               }
            }
         }
         for( const auto& producer : standby_producers )
         {
            auto existing = producer_to_last_implied_irb.find( producer );
            if( existing != producer_to_last_implied_irb.end() ) {
               new_producer_to_last_implied_irb[producer] = existing->second;
            } else {
               new_producer_to_last_implied_irb[producer] = result.dpos_irreversible_blocknum;
            }
         }

         result.producer_to_last_implied_irb = std::move( new_producer_to_last_implied_irb );

         result.was_pending_promoted = true;
      } else {
         result.active_schedule                  = active_schedule;
         result.producer_to_last_produced        = producer_to_last_produced;
         result.producer_to_last_produced[proauth.producer_name] = result.block_num;
         result.producer_to_last_implied_irb     = producer_to_last_implied_irb;
         result.producer_to_last_implied_irb[proauth.producer_name] = dpos_proposed_irreversible_blocknum;
      }

      return result;
   }

   signed_block_header pending_block_header_state::make_block_header(
                                                      const checksum256_type& transaction_mroot,
                                                      const checksum256_type& action_mroot,
                                                      const optional<producer_authority_schedule>& new_producers
   )const
   {
      signed_block_header h;

      h.timestamp         = timestamp;
      h.producer          = producer;
      h.confirmed         = confirmed;
      h.previous          = previous;
      h.transaction_mroot = transaction_mroot;
      h.action_mroot      = action_mroot;
      h.schedule_version  = active_schedule_version;

      if (new_producers) {
         // add the header extension to update the block schedule
         emplace_extension(
               h.header_extensions,
               producer_schedule_change_extension::extension_id(),
               fc::raw::pack( producer_schedule_change_extension( *new_producers ) )
         );
      }

      return h;
   }

   block_header_state pending_block_header_state::_finish_next(
                                 const signed_block_header& h
   )&&
   {
      EOS_ASSERT( h.timestamp == timestamp, block_validate_exception, "timestamp mismatch" );
      EOS_ASSERT( h.previous == previous, unlinkable_block_exception, "previous mismatch" );
      EOS_ASSERT( h.confirmed == confirmed, block_validate_exception, "confirmed mismatch" );
      EOS_ASSERT( h.producer == producer, wrong_producer, "wrong producer specified" );
      EOS_ASSERT( h.schedule_version == active_schedule_version, producer_schedule_exception, "schedule_version in signed block is corrupted" );

      auto exts = h.validate_and_extract_header_extensions();

      std::optional<producer_authority_schedule> maybe_new_producer_schedule;
      std::optional<digest_type> maybe_new_producer_schedule_hash;

      if ( exts.count(producer_schedule_change_extension::extension_id()) > 0 ) {
         EOS_ASSERT( !was_pending_promoted, producer_schedule_exception, "cannot set pending producer schedule in the same block in which pending was promoted to active" );

         const auto& new_producer_schedule = exts.lower_bound(producer_schedule_change_extension::extension_id())->second.get<producer_schedule_change_extension>();

         EOS_ASSERT( new_producer_schedule.version == active_schedule.version + 1, producer_schedule_exception, "wrong producer schedule version specified" );
         EOS_ASSERT( prev_pending_schedule.schedule.producers.empty(), producer_schedule_exception,
                     "cannot set new pending producers until last pending is confirmed" );

         maybe_new_producer_schedule_hash.emplace(digest_type::hash(new_producer_schedule));
         maybe_new_producer_schedule.emplace(new_producer_schedule);
      }

      auto block_number = block_num;

      block_header_state result( std::move( *static_cast<detail::block_header_state_common*>(this) ) );

      result.id      = h.id();
      result.header  = h;

      result.header_exts = std::move(exts);

      if( maybe_new_producer_schedule ) {
         result.pending_schedule.schedule = std::move(*maybe_new_producer_schedule);
         result.pending_schedule.schedule_hash = std::move(*maybe_new_producer_schedule_hash);
         result.pending_schedule.schedule_lib_num    = block_number;
      } else {
         if( was_pending_promoted ) {
            result.pending_schedule.schedule.version = prev_pending_schedule.schedule.version;
         } else {
            result.pending_schedule.schedule         = std::move( prev_pending_schedule.schedule );
         }
         result.pending_schedule.schedule_hash       = std::move( prev_pending_schedule.schedule_hash );
         result.pending_schedule.schedule_lib_num    = prev_pending_schedule.schedule_lib_num;
      }

      return result;
   }

   block_header_state pending_block_header_state::finish_next(
                                 const signed_block_header& h,
                                 vector<signature_type>&& additional_signatures,
                                 bool skip_validate_signee
   )&&
   {
      auto result = std::move(*this)._finish_next( h );

      if( !additional_signatures.empty() ) {
         result.additional_signatures = std::move(additional_signatures);
      }

      // ASSUMPTION FROM controller_impl::apply_block = all untrusted blocks will have their signatures pre-validated here
      if( !skip_validate_signee ) {
        result.verify_signee( );
      }

      return result;
   }

   block_header_state pending_block_header_state::finish_next(
                                 signed_block_header& h,
                                 const signer_callback_type& signer
   )&&
   {
      auto result = std::move(*this)._finish_next( h );
      result.sign( signer );
      h.producer_signature = result.header.producer_signature;

      return result;
   }

   /**
    *  Transitions the current header state into the next header state given the supplied signed block header.
    *
    *  Given a signed block header, generate the expected template based upon the header time,
    *  then validate that the provided header matches the template.
    *
    *  If the header specifies new_producers then apply them accordingly.
    */
   block_header_state block_header_state::next(
                        const signed_block_header& h,
                        vector<signature_type>&& _additional_signatures,
                        bool skip_validate_signee )const
   {
      return next( h.timestamp, h.confirmed ).finish_next( h, std::move(_additional_signatures), skip_validate_signee );
   }

   digest_type   block_header_state::sig_digest()const {
      auto header_bmroot = digest_type::hash( std::make_pair( header.digest(), blockroot_merkle.get_root() ) );
      return digest_type::hash( std::make_pair(header_bmroot, pending_schedule.schedule_hash) );
   }

   void block_header_state::sign( const signer_callback_type& signer ) {
      auto d = sig_digest();
      auto sigs = signer( d );

      EOS_ASSERT(!sigs.empty(), no_block_signatures, "Signer returned no signatures");
      header.producer_signature = sigs.back();
      sigs.pop_back();

      additional_signatures = std::move(sigs);

      verify_signee();
   }

   void block_header_state::verify_signee( )const {

      size_t num_keys_in_authority = valid_block_signing_authority.visit([](const auto &a){ return a.keys.size(); });
      EOS_ASSERT(1 + additional_signatures.size() <= num_keys_in_authority, wrong_signing_key,
                 "number of block signatures (${num_block_signatures}) exceeds number of keys in block signing authority (${num_keys})",
                 ("num_block_signatures", 1 + additional_signatures.size())
                 ("num_keys", num_keys_in_authority)
                 ("authority", valid_block_signing_authority)
      );

      std::set<public_key_type> keys;
      auto digest = sig_digest();
      keys.emplace(fc::crypto::public_key( header.producer_signature, digest, true ));

      for (const auto& s: additional_signatures) {
         auto res = keys.emplace(s, digest, true);
         EOS_ASSERT(res.second, wrong_signing_key, "block signed by same key twice", ("key", *res.first));
      }

      bool is_satisfied = false;
      size_t relevant_sig_count = 0;

      std::tie(is_satisfied, relevant_sig_count) = producer_authority::keys_satisfy_and_relevant(keys, valid_block_signing_authority);

      EOS_ASSERT(relevant_sig_count == keys.size(), wrong_signing_key,
                 "block signed by unexpected key",
                 ("signing_keys", keys)("authority", valid_block_signing_authority));

      EOS_ASSERT(is_satisfied, wrong_signing_key,
                 "block signatures do not satisfy the block signing authority",
                 ("signing_keys", keys)("authority", valid_block_signing_authority));
   }


} } /// namespace eosio::chain
