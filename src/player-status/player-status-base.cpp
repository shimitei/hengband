﻿#include "player-status/player-status-base.h"
#include "inventory/inventory-slot-types.h"
#include "object/object-flags.h"
#include "player/player-status.h"
#include "system/object-type-definition.h"
#include "util/bit-flags-calculator.h"

/*!
 * @brief プレイヤーの各ステータス計算用のクラス
 * @param owner_ptr プレイヤーの参照ポインタ
 * @details
 * * コンストラクタでowner_ptrをセット。メンバ変数を0クリア。
 */
PlayerStatusBase::PlayerStatusBase(player_type *owner_ptr)
{
    this->owner_ptr = owner_ptr;
    this->set_locals(); /* 初期化。基底クラスの0クリアが呼ばれる。*/
}

/*!
 * @brief 該当する値を計算して取得する。
 * @details
 * * 派生クラスからset_locals()をコールして初期値、上限、下限をセット。
 * * 各要素毎に計算した値を初期値に単純に加算し、上限と下限で丸める。
 */
s16b PlayerStatusBase::getValue()
{
    this->set_locals(); /* 計算前に値のセット。派生クラスの値がセットされる。*/
    s16b pow = this->default_value;

    pow += this->action_value();
    pow += this->battleform_value();
    pow += this->class_value();
    pow += this->equipments_value();
    pow += this->inventory_weight_value();
    pow += this->mutation_value();
    pow += this->personality_value();
    pow += this->race_value();
    pow += this->riding_value();
    pow += this->time_effect_value();

    if ((pow > this->max_value)) {
        pow = this->max_value;
    }

    if (pow < this->min_value)
        pow = this->min_value;

    return pow;
}

/*!
 * @brief 修正値が0でないところにビットを立てて返す。
 * @return 判定結果のBIT_FLAGS
 */
BIT_FLAGS PlayerStatusBase::getFlags()
{
    BIT_FLAGS result = equipments_flags();

    if (this->class_value() != 0)
        set_bits(result, FLAG_CAUSE_CLASS);

    if (this->race_value() != 0)
        set_bits(result, FLAG_CAUSE_RACE);

    if (this->battleform_value() != 0)
        set_bits(result, FLAG_CAUSE_BATTLE_FORM);

    if (this->mutation_value() != 0)
        set_bits(result, FLAG_CAUSE_MUTATION);

    if (this->time_effect_value() != 0)
        set_bits(result, FLAG_CAUSE_MAGIC_TIME_EFFECT);

    if (this->personality_value() != 0)
        set_bits(result, FLAG_CAUSE_PERSONALITY);

    if (this->riding_value() != 0)
        set_bits(result, FLAG_CAUSE_RIDING);

    if (this->inventory_weight_value() != 0)
        set_bits(result, FLAG_CAUSE_INVEN_PACK);

    if (this->action_value() != 0)
        set_bits(result, FLAG_CAUSE_ACTION);

    return result;
}

/*!
 * @brief 修正値が0以下のところにビットを立てて返す。
 * @return 判定結果のBIT_FLAGS
 */
BIT_FLAGS PlayerStatusBase::getBadFlags()
{
    BIT_FLAGS result = equipments_bad_flags();

    if (this->class_value() < 0)
        set_bits(result, FLAG_CAUSE_CLASS);

    if (this->race_value() < 0)
        set_bits(result, FLAG_CAUSE_RACE);

    if (this->battleform_value() < 0)
        set_bits(result, FLAG_CAUSE_BATTLE_FORM);

    if (this->mutation_value() < 0)
        set_bits(result, FLAG_CAUSE_MUTATION);

    if (this->time_effect_value() < 0)
        set_bits(result, FLAG_CAUSE_MAGIC_TIME_EFFECT);

    if (this->personality_value() < 0)
        set_bits(result, FLAG_CAUSE_PERSONALITY);

    if (this->riding_value() < 0)
        set_bits(result, FLAG_CAUSE_RIDING);

    if (this->inventory_weight_value() < 0)
        set_bits(result, FLAG_CAUSE_INVEN_PACK);

    if (this->action_value() < 0)
        set_bits(result, FLAG_CAUSE_ACTION);

    return result;
}

void PlayerStatusBase::set_locals()
{
    this->default_value = 0;
    this->min_value = 0;
    this->max_value = 0;
    this->tr_flag = TR_FLAG_MAX;
    this->tr_bad_flag = TR_FLAG_MAX;
}

BIT_FLAGS PlayerStatusBase::equipments_flags()
{
    this->set_locals(); /* 計算前に値のセット。派生クラスの値がセットされる。*/
    object_type *o_ptr;
    BIT_FLAGS flgs[TR_FLAG_SIZE];
    BIT_FLAGS result = 0L;
    for (int i = INVEN_MAIN_HAND; i < INVEN_TOTAL; i++) {
        o_ptr = &owner_ptr->inventory_list[i];
        if (!o_ptr->k_idx)
            continue;

        object_flags(owner_ptr, o_ptr, flgs);

        if (has_flag(flgs, tr_flag))
            set_bits(result, convert_inventory_slot_type_to_flag_cause(static_cast<inventory_slot_type>(i)));
    }
    return result;
}

BIT_FLAGS PlayerStatusBase::equipments_bad_flags()
{
    this->set_locals(); /* 計算前に値のセット。派生クラスの値がセットされる。*/
    object_type *o_ptr;
    BIT_FLAGS flgs[TR_FLAG_SIZE];
    BIT_FLAGS result = 0L;
    for (int i = INVEN_MAIN_HAND; i < INVEN_TOTAL; i++) {
        o_ptr = &owner_ptr->inventory_list[i];
        if (!o_ptr->k_idx)
            continue;

        object_flags(owner_ptr, o_ptr, flgs);

        if (has_flag(flgs, tr_bad_flag)) {
            if (o_ptr->pval < 0) {
                set_bits(result, convert_inventory_slot_type_to_flag_cause(static_cast<inventory_slot_type>(i)));
            }
        }
    }
    return result;
}

s16b PlayerStatusBase::equipments_value()
{
    this->set_locals(); /* 計算前に値のセット。派生クラスの値がセットされる。*/
    s16b result = 0;
    for (int i = INVEN_MAIN_HAND; i < INVEN_TOTAL; i++) {
        object_type *o_ptr = &owner_ptr->inventory_list[i];
        BIT_FLAGS flgs[TR_FLAG_SIZE];
        object_flags(owner_ptr, o_ptr, flgs);

        if (!o_ptr->k_idx)
            continue;
        if (has_flag(flgs, tr_flag))
            result += o_ptr->pval;
    }
    return result;
}

s16b PlayerStatusBase::race_value()
{
    return 0;
}
s16b PlayerStatusBase::class_value()
{
    return 0;
}
s16b PlayerStatusBase::personality_value()
{
    return 0;
}
s16b PlayerStatusBase::time_effect_value()
{
    return 0;
}
s16b PlayerStatusBase::battleform_value()
{
    return 0;
}
s16b PlayerStatusBase::mutation_value()
{
    return 0;
}
s16b PlayerStatusBase::riding_value()
{
    return 0;
}
s16b PlayerStatusBase::inventory_weight_value()
{
    return 0;
}
s16b PlayerStatusBase::action_value()
{
    return 0;
}
