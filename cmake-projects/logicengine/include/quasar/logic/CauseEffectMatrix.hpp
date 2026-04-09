/**
 * @file CauseEffectMatrix.hpp
 * @brief Implementation of IEC 62881 Cause-Effect Matrix logic.
 */

#ifndef QUASAR_LOGIC_CAUSEEFFECTMATRIX_HPP
#define QUASAR_LOGIC_CAUSEEFFECTMATRIX_HPP

#include "quasar/logic/LogicComponent.hpp"
#include "quasar/logic/BitVector.hpp"
#include <vector>

namespace quasar::logic {

/**
 * @class CauseEffectMatrix
 * @brief High-performance binary logic evaluator using bit-parallel matrices.
 * 
 * **Compliance**:
 * - Fulfills [TSK-20260311-009.4] Rule Engine & Cause-Effect Matrix (IEC 62881).
 * - Fulfills [TSK-20260311-009.4.2] Optimize AND/OR matrix evaluation.
 * - @reference [FE-0240.4] Rule Engine & Cause-Effect Matrix
 */
class CauseEffectMatrix : public LogicComponent {
public:
    /**
     * @brief Factory method.
     * @param name Component name.
     * @param causeCount Number of input causes.
     * @param effectCount Number of output effects.
     * @param parent Optional parent NamedObject.
     * @return Shared pointer to the matrix.
     */
    static std::shared_ptr<CauseEffectMatrix> create(const std::string& name, std::size_t causeCount, std::size_t effectCount, std::shared_ptr<quasar::named::NamedObject> parent = nullptr);

    /** @brief Lifecycle hooks. */
    void initialize() override;
    void start() override;
    void stop() override;
    void reset() override;

    /**
     * @brief Pauses evaluation.
     */
    void pause() override;

    /**
     * @brief Resumes evaluation.
     */
    void resume() override;

    /**
     * @brief Evaluates the matrix using the current cause state.
     * @param dt Elapsed time.
     */
    void step(duration dt) override;

    /**
     * @brief Sets the state of a cause bit.
     * @param causeIdx Index of the cause.
     * @param value Boolean state.
     */
    void setCause(std::size_t causeIdx, bool value);

    /**
     * @brief Gets the state of an effect bit.
     * @param effectIdx Index of the effect.
     * @return Boolean state.
     */
    bool getEffect(std::size_t effectIdx) const;

    /**
     * @brief Configures an AND mask for an effect.
     * @param effectIdx Index of the effect.
     * @param mask BitVector where 1 means cause must be TRUE.
     */
    void setAndMask(std::size_t effectIdx, const BitVector& mask);

    /**
     * @brief Configures an OR mask for an effect.
     * @param effectIdx Index of the effect.
     * @param mask BitVector where 1 means if any cause is TRUE, effect is TRUE.
     */
    void setOrMask(std::size_t effectIdx, const BitVector& mask);

protected:
    /**
     * @brief Constructor.
     * @param name Component name.
     * @param causeCount Number of causes.
     * @param effectCount Number of effects.
     */
    CauseEffectMatrix(const std::string& name, std::size_t causeCount, std::size_t effectCount);

private:
    /** @brief Current state of all causes. */
    BitVector m_causeVector;
    /** @brief Resulting state of all effects. */
    BitVector m_effectVector;
    /** @brief List of AND masks (one per effect). */
    std::vector<BitVector> m_andMasks;
    /** @brief List of OR masks (one per effect). */
    std::vector<BitVector> m_orMasks;

    /** @brief Flag for paused state. */
    bool m_paused{false};
};

} // namespace quasar::logic

#endif // QUASAR_LOGIC_CAUSEEFFECTMATRIX_HPP
