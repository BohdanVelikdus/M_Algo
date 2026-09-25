#ifndef CONSTEXPR_RECURSION_ALGORITHM_HPP
#define CONSTEXPR_RECURSION_ALGORITHM_HPP

#include <iostream>
#include <numeric>
#include <cassert>
#include <tuple>
#include <array>

static void fun(const int n)
{
    static int f = 0;
    if (n>0)
    {
        std::cout << &f << std::endl;
        std::cout<<n<<std::endl;
        fun(n-1);
    }
}

static void tree(const int n)
{
    if (n > 0)
    {
        printf("%d\n", n);
        tree(n-1);
        tree(n-1);
    }
}

static void FunB(int n);

static void FunA(const int n)
{
    if (n>0)
    {
        std::cout << n << std::endl;
        FunB(n-1);
    }
}

static void FunB(const int n)
{
    if (n>0)
    {
        std::cout << n << std::endl;
        FunA(n/2);
    }
}

template<std::ptrdiff_t N, typename Enable = void>
struct sum_of_natural_until;

template<std::ptrdiff_t N>
struct sum_of_natural_until<N, std::enable_if_t<(N > 0)>>
{
    static constexpr std::ptrdiff_t value = N + sum_of_natural_until<N - 1>::value;
};

template<>
struct sum_of_natural_until<0>
{
    static constexpr std::ptrdiff_t value = 0;
};

template <std::ptrdiff_t N, typename Enable = void>
struct factorial{};

template <std::ptrdiff_t N>
struct factorial<N, std::enable_if_t<(N > 0)>>
{
    static constexpr double value = N * factorial<N - 1>::value;
};

template<>
struct factorial<0>
{
    static constexpr double value = 1;
};

template<std::ptrdiff_t N>
inline constexpr std::ptrdiff_t sum_of_natural_until_v = sum_of_natural_until<N>::value;

template<std::ptrdiff_t N>
inline constexpr double factorial_v = factorial<N>::value;

// ================== Loop versions ==================

template <std::ptrdiff_t N, typename Enable = void>
struct factorial_loop{};

template <std::ptrdiff_t N>
struct factorial_loop<N, std::enable_if_t<(N > 0)>>
{
    static constexpr double value =[]<std::size_t... Idx> (std::index_sequence<Idx...>)
    {
        return (1 * ... * (Idx+1));
    }(std::make_index_sequence<N>{});
};

template<>
struct factorial_loop<0>
{
    static constexpr double value = 1;
};

template<std::ptrdiff_t N, typename Enable = void>
struct sum_of_natural_until_loop;

template<std::ptrdiff_t N>
struct sum_of_natural_until_loop<N, std::enable_if_t<(N > 0)>>
{

    static constexpr std::size_t value = []<std::size_t... Idx>(std::index_sequence<Idx...>) {
        return (0 + ... + (Idx+1));
    }(std::make_index_sequence<N>{});
};

template<>
struct sum_of_natural_until_loop<0>
{
    static constexpr std::ptrdiff_t value = 0;
};

template<std::ptrdiff_t N>
inline constexpr std::ptrdiff_t sum_of_natural_until_loop_v = sum_of_natural_until<N>::value;

template<std::ptrdiff_t N>
inline constexpr double factorial_loop_v = factorial<N>::value;

// ================== Power Ext ==================

template <std::size_t N, std::size_t P, typename Enable = void>
struct PowerToImpl;

template <std::size_t N, std::size_t P>
struct PowerToImpl<N, P, std::enable_if_t<P == 0>> {
    static constexpr double value = 1;
};

template <std::size_t N, std::size_t P>
struct PowerToImpl<N, P, std::enable_if_t<(P > 0 && P % 2 == 0)>> {
    static constexpr double value = PowerToImpl<N * N, P / 2>::value;
};

template <std::size_t N, std::size_t P>
struct PowerToImpl<N, P, std::enable_if_t<(P > 0 && P % 2 != 0)>> {
    static constexpr double value = N * PowerToImpl<N * N, (P - 1) / 2>::value;
};

template <std::size_t N, std::size_t P, typename Enable = void>
struct PowerTo{};

template <std::size_t N, std::size_t P>
struct PowerTo<N,P, std::enable_if_t<(N > 0)>>
{
    static constexpr double value = PowerToImpl<N, P>::value;
};

template <std::size_t N, std::size_t P>
inline constexpr double power_to_v = PowerTo<N,P>::value;

// ====================== Taylor recursion ======================

template <std::size_t X, std::size_t N, typename Enable = void>
struct TaylorRec{};

template <std::size_t X, std::size_t N>
struct TaylorRec<X, N, std::enable_if_t<(N>0)>>
{
    static constexpr double value = []<std::size_t... Idx>(std::index_sequence<Idx...>) {
        return ((power_to_v<X, Idx> / factorial_v<Idx>) + ...);
    }(std::make_index_sequence<N>{});
};

template <std::size_t X, std::size_t N>
inline constexpr double taylor_rec_v = TaylorRec<X, N>::value;

// ======== Taylor Horner rec =========

template <std::size_t X, std::size_t N, std::size_t K = 1>
struct HornerRec {
    static constexpr double value = 1.0 + (static_cast<double>(X) / K) * HornerRec<X, N, K + 1>::value;
};

template <std::size_t X, std::size_t N>
struct HornerRec<X, N, N> {
    static constexpr double value = 1.0;
};

template <std::size_t X, std::size_t N, typename Enable = void>
struct TaylorHornerRec{};

template <std::size_t X, std::size_t N>
struct TaylorHornerRec<X, N, std::enable_if_t<(N>0)>>
{
    static constexpr double value = HornerRec<X, N>::value;
};

template <std::size_t X, std::size_t N>
inline constexpr double taylor_horner_rec_v = TaylorHornerRec<X, N>::value;

// ====== Taylor Horner Iter

template <std::size_t X, std::size_t N, typename Enable = void>
struct TaylorHornerIter{};

template <std::size_t X, std::size_t N>
struct TaylorHornerIter<X, N, std::enable_if_t<(N>0)>>
{
    static constexpr double value = [](){
        double result = 1.0;
        for (std::size_t i = N - 1; i >= 1; --i) {
            result = 1.0 + (static_cast<double>(X) / i) * result;
        }
        return result;
    }();
};

template <std::size_t X, std::size_t N>
inline constexpr double taylor_horner_iter_v = TaylorHornerIter<X, N>::value;

// === Fib recursion

template<std::size_t N>
struct Fib
{
    static constexpr double value = Fib<N-1>::value + Fib<N-2>::value;
};

template <>
struct Fib<1>
{
    static constexpr std::size_t value = 1;
};

template <>
struct Fib<0>
{
    static constexpr std::size_t value = 0;
};

template <std::size_t N>
std::size_t fib_v = Fib<N>::value;

// ======== Combination formula

template <std::size_t N, std::size_t R, typename Enabled = void>
struct Combination
{
    static_assert(N > R && "Combination error: R cannot be greater than N.");
};

template <std::size_t N, std::size_t R>
struct Combination<N, R, std::enable_if_t<(N > R)>>
{
    static constexpr double value = factorial_loop_v<N> / (factorial_loop_v<R> * factorial_loop_v<N-R>);
};

template <std::size_t N, std::size_t R>
inline constexpr double combination_v = Combination<N, R>::value;

// ======== Tower of Hanoi ===

constexpr std::size_t A = 1;
constexpr std::size_t B = 2;
constexpr std::size_t C = 3;

constexpr std::size_t idx = A + B + C;

// A + B + C = 6
// buffer = 6 - src - dst

void han(std::size_t sizeOfTower, std::size_t src, std::size_t dst)
{
    if (sizeOfTower == 1)
    {
        std::cout << "Move: " << sizeOfTower << " From: " << src << " To: " << dst << std::endl;
    }
    else
    {
        std::size_t buffer = idx - src - dst;
        han(sizeOfTower - 1, src, buffer);

        std::cout << "Move: " << sizeOfTower << " From: " << src << " To: " << dst << std::endl;

        han(sizeOfTower - 1, buffer, dst);
    }
}

// ============ constexpr tower of Hanoi ======

using MoveTuple = std::tuple<std::size_t, std::size_t, std::size_t>;

template<std::size_t Size, std::size_t Src, std::size_t Dst>
struct HanCs
{
    static constexpr auto value = []()
    {
        constexpr std::size_t buffer = idx - Src - Dst;
        auto bArr = HanCs<Size-1, Src, buffer>::value;
        MoveTuple rs = std::make_tuple(1, Src, Dst);
        auto aArr = HanCs<Size-1, buffer, Dst>::value;
        std::array<MoveTuple, bArr.size() + 1 + aArr.size()> resArray{};
        std::copy(bArr.begin(), bArr.end(), resArray.begin());
        resArray[bArr.size()] = rs;
        std::copy(aArr.begin(), aArr.end(), resArray.begin() + bArr.size() + 1);
        return resArray;
    }();
};

template<std::size_t Src, std::size_t Dst>
struct HanCs<1, Src, Dst>
{
    static constexpr std::array<MoveTuple, 1> value = { MoveTuple{1, Src, Dst} };
};

template<std::size_t Size, std::size_t Src, std::size_t Dst>
auto han_cs_v = HanCs<Size, Src, Dst>::value;


#endif
