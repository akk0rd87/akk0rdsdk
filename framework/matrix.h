#ifndef __AKK0RD_MATRIX_H__
#define __AKK0RD_MATRIX_H__

#include <functional>
template <class T>
class Matrix {
private:
    std::vector<T> Data;
    int w, h;
public:
    Matrix() : w{ 0 }, h{ 0 } { };
    Matrix(int W, int H, const T& Value) { Fill(W, H, Value); }
    void Clear() { Data.clear(); w = h = 0; };
    void Resize(int W, int H) { Data.resize(W * H); w = W; h = H; }
    void Fill(int W, int H, const T& Value) { Data.clear(); Data.insert(Data.begin(), W * H, Value); w = W; h = H; };
    void Reserve(int Count) { Data.reserve(Count); }
    void Reserve(int W, int H) { Data.reserve(W * H); }
    /* */ T& operator[](int Index) /* */ { return Data[Index]; }
    const T& operator[](int Index) const { return Data[Index]; }
    /* */ T& operator()(int I, int J) /* */ { return Data[J * w + I]; }
    const T& operator()(int I, int J) const { return Data[J * w + I]; }
    int GetW() const { return w; }
    int GetH() const { return h; }
    void ForEach(const std::function<void(T& Value)>& Func) {
        std::for_each(Data.begin(), Data.end(), Func);
    };

    /*
    class Line {
    private:
        const Matrix<T>* matrix;
        decltype(matrix->Data.begin()) line_begin;
    public:
        Line(const Matrix<T>* Matrix, decltype(matrix->Data.begin()) Begin) : matrix{ Matrix }, line_begin{ Begin }{  };
        class forward_line_iterator {
        private:
            decltype(matrix->Data.begin()) current;
        public:
            forward_line_iterator(decltype(matrix->Data.begin()) Pointer) : current{ Pointer } { }
            const T& operator*() const { return *current; };
            //T& operator*() { return *current; };
            bool operator==(const forward_line_iterator& b) const { return b.current == current; };
            bool operator!=(const forward_line_iterator& b) const { return b.current != current; }
            forward_line_iterator& operator++() { ++current; return *this; }
        };

        forward_line_iterator begin() { return forward_line_iterator(line_begin); };
        forward_line_iterator end() { return forward_line_iterator(line_begin + this->matrix->GetW()); };

        const forward_line_iterator begin() const { return cbegin(); };
        const forward_line_iterator end() const { return cend(); };

        const forward_line_iterator cbegin() const { return forward_line_iterator(line_begin); };
        const forward_line_iterator cend() const { return forward_line_iterator(line_begin + this->matrix->GetW()); };

        int Order() const { return static_cast<int>(std::distance(matrix->Data.begin(), line_begin)) / this->matrix->GetW(); }
    };

    class Lines {
    private:
        const Matrix<T>* matrix;
    public:
        Lines(const Matrix<T>* matrix) : matrix{ matrix } {  };
        class forward_lines_iterator {
        private:
            const Matrix<T>* matrix;
            decltype(matrix->Data.begin()) current;
        public:
            forward_lines_iterator(const Matrix<T>* Matrix, decltype(matrix->Data.begin()) Pointer) : matrix{ Matrix }, current(Pointer) {  }
            Line operator*() const { return Line(this->matrix, current); };
            bool operator==(const forward_lines_iterator& b) const { return b.current == current; };
            bool operator!=(const forward_lines_iterator& b) const { return b.current != current; }
            forward_lines_iterator& operator++() { current += matrix->GetW(); return *this; }
        };
        forward_lines_iterator begin() { return forward_lines_iterator(this->matrix, this->matrix->Data.begin()); };
        forward_lines_iterator end() { return forward_lines_iterator(this->matrix, this->matrix->Data.end()); };

        const forward_lines_iterator begin() const { return cbegin(); };
        const forward_lines_iterator end() const { return cend(); };

        const forward_lines_iterator cbegin() const { return forward_lines_iterator(this->matrix, this->matrix->Data.begin()); };
        const forward_lines_iterator cend() const { return forward_lines_iterator(this->matrix, this->matrix->Data.end()); };
    };

    Lines GetLines() { return Lines(this); };
    const Lines GetLines() const { return Lines(this); };
    */
};

#endif // __AKK0RD_MATRIX_H__