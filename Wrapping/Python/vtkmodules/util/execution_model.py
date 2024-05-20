"""Utility classes to help with the simpler Python interface
for connecting and executing pipelines."""

def _call(first, last, inp=None, port=0):
    """Set the input of the first filter, update the pipeline
    and return the output."""
    print("Port:" , port)
    if inp and not first.GetNumberOfInputPorts():
        raise ValueError(f"{first.GetClassName()} does not have input ports yet an input was passed to the pipeline.")
    in_cons = []
    if first.GetNumberOfInputPorts():
        n_cons = first.GetNumberOfInputConnections(port)
        for i in range(n_cons):
            op = first.GetInputConnection(port, i)
            if op and op.GetProducer():
                op.GetProducer().Register(None)
            in_cons.append(op)
        from vtkmodules.vtkCommonExecutionModel import vtkAlgorithm
        from vtkmodules.vtkCommonExecutionModel import vtkTrivialProducer
        from collections.abc import Sequence
        if isinstance(inp, Sequence):
            if first.GetInputPortInformation(port).Has(
                    vtkAlgorithm.INPUT_IS_REPEATABLE()):
                first.RemoveAllInputConnections(port)
                for aInp in inp:
                    tp = vtkTrivialProducer()
                    tp.SetOutput(aInp)
                    first.AddInputConnection(port, tp.GetOutputPort());
        else:
            tp = vtkTrivialProducer()
            tp.SetOutput(inp)
            first.SetInputConnection(port, tp.GetOutputPort());

    output = last.update().output

    if first.GetNumberOfInputPorts():
        first.RemoveAllInputConnections(port)
        for op in in_cons:
            first.AddInputConnection(port, op)
            if op and op.GetProducer():
                op.GetProducer().UnRegister(None)

    output_copy = []
    if type(output) is not tuple:
        output = (output,)
    for opt in output:
        copy = opt.NewInstance()
        copy.ShallowCopy(opt)
        output_copy.append(copy)
    if len(output_copy) == 1:
        return output_copy[0]
    else:
        return tuple(output_copy)


class select_ports(object):
    """Helper class for selecting input and output ports when
    connecting pipeline objects with the >> operator.
    Example uses:
    # Connect a source to the second input of a filter.
    source >> select_ports(1, filter)
    # Connect the second output of a source to a filter.
    select_ports(source, 1) >> filter
    # Combination of both: Connect source to second
    # input of the filter, then connect the second
    # output of that filter to another one.
    source >>> select_ports(1, filter, 1) >> filter2
    """
    def __init__(self, *args):
        """This constructor takes 2 or 3 arguments.
        The possibilities are:
        select_ports(input_port, algorithm)
        select_ports(algorithm, output_port)
        select_ports(input_port, algorithm, output_port)
        """
        nargs = len(args)
        if nargs < 2 or nargs > 3:
            raise ValueError("Expecting 2 or 3 arguments")

        self.input_port = None
        self.output_port = None
        before_alg = True
        for arg in args:
            if hasattr(arg, "IsA") and arg.IsA("vtkAlgorithm"):
                self.algorithm = arg
                before_alg = False
            else:
                if before_alg:
                    self.input_port = arg
                else:
                    self.output_port = arg
        if not self.input_port:
            self.input_port = 0
        if not self.output_port:
            self.output_port = 0

    def SetInputConnection(self, inp):
        self.algorithm.SetInputConnection(self.input_port, inp)

    def GetOutputPort(self):
        return self.algorithm.GetOutputPort(self.output_port)

    def update(self):
        """Execute the algorithm and return the output from the selected
        output port."""
        return self.algorithm.update()

    def __rshift__(self, rhs):
        return Pipeline(self, rhs)

    def __call__(self, inp=None):
        return _call(self.algorithm, self.algorithm, inp, self.input_port)

class Pipeline(object):
    """Pipeline objects are created when 2 or more algorithms are
    connected with the >> operator. They store the first and last
    algorithms in the pipeline and enable connecting more algorithms
    and executing the pipeline. One should not have to create Pipeline
    objects directly. They are created by the use of the >> operator."""

    PIPELINE = 0
    ALGORITHM = 1
    DATA = 2
    UNKNOWN = 3

    def __init__(self, lhs, rhs):
        """Create a pipeline object that connects two objects of the
        following type: data object, pipeline object, algorithm object."""

        left_type = self._determine_type(lhs)
        right_type = self._determine_type(rhs)
        if left_type == Pipeline.UNKNOWN or right_type == Pipeline.UNKNOWN:
            raise TypeError(
                    f"unsupported operand type(s) for >>: {type(lhs).__name__} and {type(rhs).__name__}")
        if right_type == Pipeline.ALGORITHM:
            if left_type == Pipeline.ALGORITHM:
                rhs.SetInputConnection(lhs.GetOutputPort())
                self.first = lhs
                self.last = rhs
            elif left_type == Pipeline.PIPELINE:
                rhs.SetInputConnection(lhs.last.GetOutputPort())
                self.first = lhs.first
                self.last = rhs
            elif left_type == Pipeline.DATA:
                from vtkmodules.vtkCommonExecutionModel import vtkTrivialProducer
                source = vtkTrivialProducer()
                source.SetOutput(lhs)
                rhs.SetInputConnection(source.GetOutputPort())
                self.first = source
                self.last = rhs
        elif right_type == Pipeline.PIPELINE:
            if left_type == Pipeline.ALGORITHM:
                self.first = lhs
                self.last = rhs.last
                rhs.first.SetInputConnection(lhs.GetOutputPort())
            elif left_type == Pipeline.PIPELINE:
                rhs.first.SetInputConnection(lhs.last.GetOutputPort())
                self.first = lhs.first
                self.last = rhs.last
            elif left_type == Pipeline.DATA:
                from vtkmodules.vtkCommonExecutionModel import vtkTrivialProducer
                source = vtkTrivialProducer()
                source.SetOutput(lhs)
                rhs.first.SetInputConnection(source.GetOutputPort())
                self.first = source
                self.last = rhs.last

    def _determine_type(self, arg):
        if type(arg) is Pipeline:
            return Pipeline.PIPELINE
        if hasattr(arg, "SetInputConnection"):
            return Pipeline.ALGORITHM
        if hasattr(arg, "IsA") and arg.IsA("vtkDataObject"):
            return Pipeline.DATA
        return Pipeline.UNKNOWN

    def update(self, **kwargs):
        """Update the pipeline and return the last algorithm's
        output."""
        return self.last.update()

    def __call__(self, inp=None):
        """Set the input of the first filter, update the pipeline
        and return the output."""
        return _call(self.first, self.last, inp)

    def __rshift__(self, rhs):
        return Pipeline(self, rhs)

class Output(object):
    def __init__(self, algorithm, **kwargs):
        self.algorithm = algorithm
        self.algorithm.Update()

    @property
    def output(self):
        if self.algorithm.GetNumberOfOutputPorts() == 1:
            return self.algorithm.GetOutputDataObject(0)
        else:
            outputs = []
            nOutputs = self.algorithm.GetNumberOfOutputPorts()
            for i in range(nOutputs):
                outputs.append(self.algorithm.GetOutputDataObject(i))
        return tuple(outputs)
