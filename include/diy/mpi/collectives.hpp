#include <vector>

#include "operations.hpp"

namespace diy
{
namespace mpi
{
  template<class T, class Op>
  struct Collectives
  {
    typedef   detail::mpi_datatype<T>     Datatype;

    static void broadcast(const communicator& comm, T& x, int root)
    {
      MPI_Bcast(Datatype::address(x),
                Datatype::count(x),
                Datatype::datatype(), root, comm);
    }

    static void gather(const communicator& comm, const T& in, std::vector<T>& out, int root)
    {
      size_t s  = comm.size();
             s *= Datatype::count(in);
      out.resize(s);
      MPI_Gather(Datatype::address(in),
                 Datatype::count(in),
                 Datatype::datatype(),
                 Datatype::address(out[0]),
                 Datatype::count(in),
                 Datatype::datatype(),
                 root, comm);
    }

    static void gather(const communicator& comm, const std::vector<T>& in, std::vector< std::vector<T> >& out, int root)
    {
      std::vector<int>  counts(comm.size());
      Collectives<int,void*>::gather(comm, (int) in.size(), counts, root);

      std::vector<int>  offsets(comm.size(), 0);
      for (unsigned i = 1; i < offsets.size(); ++i)
        offsets[i] = offsets[i-1] + counts[i-1];

      std::vector<T> buffer(offsets.back() + counts.back());
      MPI_Gatherv(Datatype::address(in[0]),
                  in.size(),
                  Datatype::datatype(),
                  Datatype::address(buffer[0]),
                  &counts[0],
                  &offsets[0],
                  Datatype::datatype(),
                  root, comm);

      out.resize(comm.size());
      size_t cur = 0;
      for (unsigned i = 0; i < comm.size(); ++i)
      {
          out[i].reserve(counts[i]);
          for (unsigned j = 0; j < counts[i]; ++j)
              out[i].push_back(buffer[cur++]);
      }
    }

    static void gather(const communicator& comm, const T& in, int root)
    {
      MPI_Gather(Datatype::address(in),
                 Datatype::count(in),
                 Datatype::datatype(),
                 Datatype::address(const_cast<T&>(in)),
                 Datatype::count(in),
                 Datatype::datatype(),
                 root, comm);
    }

    static void gather(const communicator& comm, const std::vector<T>& in, int root)
    {
      Collectives<int,void*>::gather(comm, (int) in.size(), root);

      MPI_Gatherv(Datatype::address(in[0]),
                  in.size(),
                  Datatype::datatype(),
                  0, 0, 0,
                  Datatype::datatype(),
                  root, comm);
    }

    static void reduce(const communicator& comm, const T& in, T& out, int root, const Op&)
    {
      MPI_Reduce(Datatype::address(in),
                 Datatype::address(out),
                 Datatype::count(in),
                 Datatype::datatype(),
                 detail::mpi_op<Op>::get(),
                 root, comm);
    }

    static void reduce(const communicator& comm, const T& in, int root, const Op& op)
    {
      MPI_Reduce(Datatype::address(in),
                 Datatype::address(const_cast<T&>(in)),
                 Datatype::count(in),
                 Datatype::datatype(),
                 detail::mpi_op<Op>::get(),
                 root, comm);
    }

    static void all_reduce(const communicator& comm, const T& in, T& out, const Op&)
    {
      MPI_Allreduce(Datatype::address(in),
                    Datatype::address(out),
                    Datatype::count(in),
                    Datatype::datatype(),
                    detail::mpi_op<Op>::get(),
                    comm);
    }

    static void scan(const communicator& comm, const T& in, T& out, const Op&)
    {
      MPI_Scan(Datatype::address(in),
               Datatype::address(out),
               Datatype::count(in),
               Datatype::datatype(),
               detail::mpi_op<Op>::get(),
               comm);
    }

    static void all_to_all(const communicator& comm, const std::vector<T>& in, std::vector<T>& out, int n = 1)
    {
      // NB: this will fail if T is a vector
      MPI_Alltoall(Datatype::address(in[0]), n,
                   Datatype::datatype(),
                   Datatype::address(out[0]), n,
                   Datatype::datatype(),
                   comm);
    }
  };

  template<class T>
  void      broadcast(const communicator& comm, T& x, int root)
  {
    Collectives<T,void*>::broadcast(comm, x, root);
  }

  template<class T>
  void      gather(const communicator& comm, const T& in, std::vector<T>& out, int root)
  {
    Collectives<T,void*>::gather(comm, in, out, root);
  }

  template<class T>
  void      gather(const communicator& comm, const std::vector<T>& in, std::vector< std::vector<T> >& out, int root)
  {
    Collectives<T,void*>::gather(comm, in, out, root);
  }

  template<class T>
  void      gather(const communicator& comm, const T& in, int root)
  {
    Collectives<T,void*>::gather(comm, in, root);
  }

  template<class T>
  void      gather(const communicator& comm, const std::vector<T>& in, int root)
  {
    Collectives<T,void*>::gather(comm, in, root);
  }

  template<class T, class Op>
  void      reduce(const communicator& comm, const T& in, T& out, int root, const Op& op)
  {
    Collectives<T, Op>::reduce(comm, in, out, root, op);
  }

  // Should not be called on the root process
  template<class T, class Op>
  void      reduce(const communicator& comm, const T& in, int root, const Op& op)
  {
    Collectives<T, Op>::reduce(comm, in, root, op);
  }

  template<class T, class Op>
  void      all_reduce(const communicator& comm, const T& in, T& out, const Op& op)
  {
    Collectives<T, Op>::all_reduce(comm, in, out, op);
  }

  template<class T, class Op>
  void      scan(const communicator& comm, const T& in, T& out, const Op& op)
  {
    Collectives<T, Op>::scan(comm, in, out, op);
  }

  template<class T>
  void      all_to_all(const communicator& comm, const std::vector<T>& in, std::vector<T>& out, int n = 1)
  {
    Collectives<T, void*>::all_to_all(comm, in, out, n);
  }
}
}
