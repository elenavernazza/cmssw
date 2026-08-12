# taken from Sam James Harper: https://gitlab.cern.ch/sharper/HLTAnalyserPy

from DataFormats.FWLite import Events, Handle
import six

class HandleData(Handle):
    def __init__(self, product, label):
        Handle.__init__(self, product)
        self.label = str(label)

    def get(self, event):
        event.getByLabel(self.label, self)

    def get_label(self, split=True):
        if split:
            parts = self.label.split(":")
            while len(parts) < 3:
                parts.append("")
            return parts[0], parts[1], parts[2]
        else:
            return str(self.label)

class EvtHandles:
    def __init__(self, products=[], verbose=False):
        for product in products:
            if verbose:
                print(f"Adding handle {product['name']}, {product['type']}, {product['tag']}")
            setattr(self, product['name'], HandleData(product['type'], product['tag']))

class EvtData:
    def __init__(self, products=[], verbose=False):
        self.handles = EvtHandles(products, verbose)
        self.event = None
        self.got_handles = []

    def get_handles(self, event, on_demand=True):
        self.got_handles = []
        self.event = event
        if not on_demand:
            for name, handle in six.iteritems(vars(self.handles)):
                handle.get(event)
                self.got_handles.append(name)

    def get_handle(self, name):
        handle = getattr(self.handles, name)
        if name not in self.got_handles:
            handle.get(self.event)
            self.got_handles.append(name)
        return handle

    def get(self, name):
        handle = self.get_handle(name)
        try:
            return handle.product()
        except RuntimeError:
            return None

def add_product(prods, name, type_, tag):
    prods.append({'name': name, 'type': type_, 'tag': tag})
