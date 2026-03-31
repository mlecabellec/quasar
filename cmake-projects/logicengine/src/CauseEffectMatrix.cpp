/**
 * @file CauseEffectMatrix.cpp
 * @brief Implementation of Cause-Effect Matrix.
 */

#include "quasar/logic/CauseEffectMatrix.hpp"
#include <stdexcept>

namespace quasar::logic {

CauseEffectMatrix::CauseEffectMatrix(const std::string& name, std::size_t causeCount, std::size_t effectCount)
    : LogicComponent(name), m_causeVector(causeCount), m_effectVector(effectCount) {
    m_andMasks.resize(effectCount, BitVector(causeCount));
    m_orMasks.resize(effectCount, BitVector(causeCount));
}

std::shared_ptr<CauseEffectMatrix> CauseEffectMatrix::create(const std::string& name, std::size_t causeCount, std::size_t effectCount, std::shared_ptr<quasar::named::NamedObject> parent) {
    struct Helper : public CauseEffectMatrix {
        Helper(const std::string& n, std::size_t c, std::size_t e) : CauseEffectMatrix(n, c, e) {}
    };
    std::shared_ptr<Helper> matrix = std::make_shared<Helper>(name, causeCount, effectCount);
    matrix->setSelf(matrix);
    if (parent) {
        matrix->setParent(parent);
    }
    return matrix;
}

void CauseEffectMatrix::initialize() {
    setState(quasar::named::EntityState::Ready);
}

void CauseEffectMatrix::start() {
    setState(quasar::named::EntityState::Running);
}

void CauseEffectMatrix::stop() {
    setState(quasar::named::EntityState::Ready);
}

void CauseEffectMatrix::reset() {
    m_causeVector.clear();
    m_effectVector.clear();
    setState(quasar::named::EntityState::Ready);
}

void CauseEffectMatrix::pause() {
    m_paused = true;
}

void CauseEffectMatrix::resume() {
    m_paused = false;
}

void CauseEffectMatrix::step(duration /*dt*/) {
    if (m_paused) return;

    // [TSK-20260311-009.4.2] Optimize AND/OR matrix evaluation.
    // Effect[i] = ((Inputs & AndMask[i]) == AndMask[i]) | ((Inputs & OrMask[i]) != 0)
    
    for (std::size_t i = 0; i < m_effectVector.size(); ++i) {
        const BitVector& andMask = m_andMasks[i];
        const BitVector& orMask = m_orMasks[i];
        
        bool andResult = false;
        if (andMask.any()) {
            andResult = (m_causeVector & andMask) == andMask;
        }
        
        bool orResult = false;
        if (orMask.any()) {
            orResult = (m_causeVector & orMask).any();
        }
        
        m_effectVector.set(i, andResult || orResult);
    }
}

void CauseEffectMatrix::setCause(std::size_t causeIdx, bool value) {
    m_causeVector.set(causeIdx, value);
}

bool CauseEffectMatrix::getEffect(std::size_t effectIdx) const {
    return m_effectVector.get(effectIdx);
}

void CauseEffectMatrix::setAndMask(std::size_t effectIdx, const BitVector& mask) {
    if (effectIdx >= m_andMasks.size()) {
        throw std::out_of_range("Effect index out of range");
    }
    m_andMasks[effectIdx] = mask;
}

void CauseEffectMatrix::setOrMask(std::size_t effectIdx, const BitVector& mask) {
    if (effectIdx >= m_orMasks.size()) {
        throw std::out_of_range("Effect index out of range");
    }
    m_orMasks[effectIdx] = mask;
}

} // namespace quasar::logic
