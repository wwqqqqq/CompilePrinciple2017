'''
Python3
'''
import antlr4
import MultFirstLexer
import MultFirstParser
import MultFirstListener
import PlusFirstLexer
import PlusFirstParser
import PlusFirstListener
from typing import Mapping

class MFListener(MultFirstListener.MultFirstListener):
    def __init__(self, var_value_source: Mapping[str, int]):
        self.var_value_source = var_value_source
    def exitMult(self, ctx: MultFirstParser.MultFirstParser.MultContext):
        ctx.value = ctx.getChild(0).value * ctx.getChild(2).value
    def exitNum(self, ctx: MultFirstParser.MultFirstParser.NumContext) :
        ctx.value = int(str(ctx.getChild(0)))
    def exitId(self, ctx: MultFirstParser.MultFirstParser.IdContext):
        ctx.value = self.var_value_source[str(ctx.getChild(0))]
    def exitPlus(self, ctx: MultFirstParser.MultFirstParser.PlusContext):
        ctx.value = ctx.getChild(0).value + ctx.getChild(2).value

class PFListener(PlusFirstListener.PlusFirstListener):
    def __init__(self, var_value_source: Mapping[str, int]):
        self.var_value_source = var_value_source
    def exitPlus(self, ctx: PlusFirstParser.PlusFirstParser.PlusContext):
        ctx.value = ctx.getChild(0).value + ctx.getChild(2).value
    def exitMult(self, ctx: PlusFirstParser.PlusFirstParser.MultContext):
        ctx.value = ctx.getChild(0).value * ctx.getChild(2).value
    def exitNum(self, ctx: PlusFirstParser.PlusFirstParser.NumContext):
        ctx.value = int(str(ctx.getChild(0)))
    def exitId(self, ctx: PlusFirstParser.PlusFirstParser.IdContext):
        ctx.value = self.var_value_source[str(ctx.getChild(0))]

class LazyInputDict(dict):
    def __getitem__(self, key):
        try:
            return dict.__getitem__(self, key)
        except KeyError:
            self[key] = int(
                    input('Please enter value for variable \'{}\':'.format(key)))
            return dict.__getitem__(self, key)

if __name__ == '__main__':
    expression = input('Please enter an expression: ')
    PARSER = MultFirstParser.MultFirstParser(antlr4.CommonTokenStream(MultFirstLexer.MultFirstLexer(
        antlr4.InputStream(expression))))
    PARSER.addParseListener(MFListener(LazyInputDict()))
    print('MultFirst value: ',PARSER.expr().value)
    PARSER = PlusFirstParser.PlusFirstParser(antlr4.CommonTokenStream(PlusFirstLexer.PlusFirstLexer(
        antlr4.InputStream(expression))))
    PARSER.addParseListener(PFListener(LazyInputDict()))
    print('PlusFirst value: ',PARSER.expr().value)
