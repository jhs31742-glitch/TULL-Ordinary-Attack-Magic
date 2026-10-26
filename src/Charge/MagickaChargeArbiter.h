#pragma once

#include "Charge/ChargeSession.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <span>

namespace RE
{
    class Actor;
}

namespace TOAM::Charge
{
    inline constexpr std::size_t kMaxDrainRequests = 8;

    struct DrainRequest
    {
        ChargeSession* session{ nullptr };
        float requested{ 0.0F };
    };

    struct DrainAllocation
    {
        ChargeSession* session{ nullptr };
        float requested{ 0.0F };
        float allocated{ 0.0F };
    };

    struct AllocationPlan
    {
        std::array<DrainAllocation, kMaxDrainRequests> entries{};

        std::size_t count{ 0 };

        float available{ 0.0F };
        float totalRequested{ 0.0F };
        float totalAllocated{ 0.0F };

        bool constrained{ false };
        bool truncated{ false };
    };

    struct CommitResult
    {
        AllocationPlan plan{};

        bool committed{ false };

        float magickaBefore{ 0.0F };
        float magickaAfter{ 0.0F };

        float totalAppliedToSessions{ 0.0F };
    };

    [[nodiscard]]
    inline AllocationPlan ResolveProportional(
        const std::span<const DrainRequest> requests,
        const float available) noexcept
    {
        AllocationPlan plan{};

        if (
            std::isfinite(available) &&
            available > 0.0F
        ) {
            plan.available = available;
        }

        const std::size_t requestCount =
            requests.size() < kMaxDrainRequests ?
            requests.size() :
            kMaxDrainRequests;

        plan.count = requestCount;
        plan.truncated =
            requests.size() > kMaxDrainRequests;

        double totalRequested = 0.0;
        std::size_t lastValid =
            kMaxDrainRequests;

        for (std::size_t i = 0; i < requestCount; ++i) {

            const auto& input =
                requests[i];

            auto& output =
                plan.entries[i];

            output.session =
                input.session;

            if (!input.session) {
                continue;
            }

            if (!std::isfinite(input.requested)) {
                continue;
            }

            if (input.requested <= 0.0F) {
                continue;
            }

            output.requested =
                input.requested;

            totalRequested +=
                static_cast<double>(
                    input.requested
                );

            lastValid = i;
        }

        plan.totalRequested =
            static_cast<float>(
                totalRequested
            );

        if (plan.truncated) {
            return plan;
        }

        if (totalRequested <= 0.0) {
            return plan;
        }

        if (plan.available <= 0.0F) {
            return plan;
        }

        const double budget =
            static_cast<double>(
                plan.available
            );

        if (budget >= totalRequested) {

            double totalAllocated = 0.0;

            for (std::size_t i = 0; i < requestCount; ++i) {

                auto& entry =
                    plan.entries[i];

                entry.allocated =
                    entry.requested;

                totalAllocated +=
                    static_cast<double>(
                        entry.allocated
                    );
            }

            plan.totalAllocated =
                static_cast<float>(
                    totalAllocated
                );

            return plan;
        }

        plan.constrained = true;

        const double ratio =
            budget / totalRequested;

        double runningAllocated = 0.0;

        for (std::size_t i = 0; i < requestCount; ++i) {

            auto& entry =
                plan.entries[i];

            if (entry.requested <= 0.0F) {
                continue;
            }

            double allocation = 0.0;

            if (i == lastValid) {

                allocation =
                    budget -
                    runningAllocated;
            }
            else {

                allocation =
                    static_cast<double>(
                        entry.requested
                    ) *
                    ratio;

                const double remaining =
                    budget -
                    runningAllocated;

                if (allocation > remaining) {
                    allocation = remaining;
                }
            }

            if (allocation < 0.0) {
                allocation = 0.0;
            }

            if (
                allocation >
                static_cast<double>(
                    entry.requested
                )
            ) {
                allocation =
                    static_cast<double>(
                        entry.requested
                    );
            }

            entry.allocated =
                static_cast<float>(
                    allocation
                );

            runningAllocated +=
                static_cast<double>(
                    entry.allocated
                );
        }

        double finalSum = 0.0;

        for (std::size_t i = 0; i < requestCount; ++i) {
            finalSum +=
                static_cast<double>(
                    plan.entries[i].allocated
                );
        }

        if (
            lastValid < requestCount &&
            finalSum < budget
        ) {
            const double residue =
                budget - finalSum;

            auto& last =
                plan.entries[lastValid];

            double corrected =
                static_cast<double>(
                    last.allocated
                ) +
                residue;

            if (
                corrected >
                static_cast<double>(
                    last.requested
                )
            ) {
                corrected =
                    static_cast<double>(
                        last.requested
                    );
            }

            last.allocated =
                static_cast<float>(
                    corrected
                );
        }

        finalSum = 0.0;

        for (std::size_t i = 0; i < requestCount; ++i) {
            finalSum +=
                static_cast<double>(
                    plan.entries[i].allocated
                );
        }

        plan.totalAllocated =
            static_cast<float>(
                finalSum
            );

        if (plan.totalAllocated > plan.available) {
            plan.totalAllocated =
                plan.available;
        }

        return plan;
    }

    [[nodiscard]]
    CommitResult CommitChargeDrain(
        RE::Actor* actor,
        std::span<const DrainRequest> requests) noexcept;
}