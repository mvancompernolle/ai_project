#include <misc/observer.h>

#include <vector>
#include <iostream>
using namespace std;

/*
    Subject-Observer Pattern Example

    This example related to a spread-sheet consisting of
    Cell objects that can be inter-dependent.  Each cell is
    both a potential subject and observer.  Potentially,
    a cyclic dependency can exist between a sequence of
    cells.

    In this example, each Cell stores a value (_val) and
    CellSum cells also stores a list of other cells which
    are summed to determine the value.  CellSum is notified
    whenever one of these cells is updated.

    The implementation of subject-observer here does not
    handle cyclic dependencies.  Perhaps a new combined
    Subject-Observer class can be designed to handle this
    case.
*/

class Cell : public GltSubject
{
public:

    Cell()
    : _val(0.0)
    {
    }

    void set(double val)
    {
        _val = val;
        notify(this);
    }

    double _val;
};

class CellSum : public Cell, public GltObserver<Cell>
{
public:

    void add(Cell *ptr)
    {
        observe(*ptr);
        _cells.push_back(ptr);
    }

    void calc()
    {
        _val = 0.0;
        for (uint32 i=0; i<_cells.size(); i++)
            _val += _cells[i]->_val;
    }

    void OnNotify(const Cell *ptr)
    {
        calc();
        cout << "Sum has been updated to " << _val << endl;
        notify(this);
    }

private:
    vector<const Cell *> _cells;
};

int main(int argc,char *argv[])
{
    Cell a,b,c;
    CellSum sum;

    sum.add(&a);
    sum.add(&b);
    sum.add(&c);

    a.set(1.0);
    b.set(1.0);
    c.set(1.0);

    return EXIT_SUCCESS;
}
