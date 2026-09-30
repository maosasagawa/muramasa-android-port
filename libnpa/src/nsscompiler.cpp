#include <cstring>
#include "nsscompiler.hpp"
#include "scriptfile.hpp"
#include "flex.hpp"
#include "parser.hpp"
#include "buffer.hpp"
using namespace Npa;

class Program;
extern Program* pRoot;

// TODO: if/while/select have separate counters
static uint32_t SymCounter;
static uint32_t Counter;
static Buffer* Output;
static Buffer* MapOutput;

const char* ArgumentTypes[] =
{
    "INT",
    "STRING",
    "FLOAT"
};

Call* MakeCall(string Name, uint16_t Magic)
{
    ExpressionList Args;
    Args.push_back(new Argument(Name, ARG_STRING));
    Argument* Arg = new Argument(Nsb::StringifyMagic(Magic), ARG_FUNCTION);
    return new Call(Arg, Args, Magic);
}

void WriteSymbol(const string& Symbol)
{
    MapOutput->Write<uint32_t>(Output->GetSize());
    MapOutput->WriteStr16(Symbol);
}

void Node::Compile(uint16_t Magic, uint16_t NumParams)
{
    Output->Write<uint32_t>(Counter);
    Output->Write<uint16_t>(Magic);
    Output->Write<uint16_t>(NumParams);
    ++Counter;
}

void Argument::Compile()
{
    if (Type == ARG_VARIABLE)
        Node::Compile(MAGIC_VARIABLE, 1);
    else
    {
        Node::Compile(MAGIC_LITERAL, 2);
        Output->WriteStr32(NpaFile::FromUtf8(ArgumentTypes[Type]));
    }
    CompileRaw();
}

void Argument::CompileRaw()
{
    Output->WriteStr32(Data);
}

void Array::Compile()
{
    for (auto i = Arguments.begin(); i != Arguments.end(); ++i)
        (*i)->Compile();

    Node::Compile(MAGIC_SUB_SCRIPT, 2);
    Argument Arg1(ArrayData, ARG_STRING);
    Arg1.CompileRaw();
    Argument Arg2(to_string(Arguments.size()), ARG_INT);
    Arg2.CompileRaw();
}

void Expression::CompileRaw()
{
    Output->WriteStr32("@");
}

void Call::Compile()
{
    uint16_t NumParams = Arguments.size();
    uint32_t BuiltinMagic = Nsb::MagicifyString(Name->Data.c_str());

    // Parameters
    if (BuiltinMagic != MAGIC_PARSE_TEXT &&
        BuiltinMagic != MAGIC_CALL_SCENE &&
        BuiltinMagic != MAGIC_CALL_CHAPTER)
        for (auto i = Arguments.begin(); i != Arguments.end(); ++i)
            (*i)->Compile();

    // Builtin function
    if (BuiltinMagic != 0)
        Node::Compile(BuiltinMagic, NumParams);
    // Script function
    else
    {
        NumParams += 1;
        Node::Compile(Magic, NumParams);
        Name->CompileRaw();
    }
    // Arguments
    for (auto i = Arguments.begin(); i != Arguments.end(); ++i)
        (*i)->CompileRaw();
}

void CallStatement::Compile()
{
    Call::Compile();
    Node::Compile(MAGIC_CLEAR_PARAMS, 0);
}

void Block::Compile()
{
    Node::Compile(MAGIC_SCOPE_BEGIN, 0);
    for (auto i = Statements.begin(); i != Statements.end(); ++i)
        (*i)->Compile();
    Node::Compile(MAGIC_SCOPE_END, 0);
}

void Subroutine::CompilePrototype(uint16_t BeginMagic, uint32_t NumBeginParams)
{
    WriteSymbol(Name->Data);
    Node::Compile(BeginMagic, NumBeginParams);
    Name->CompileRaw();
}

void Subroutine::Compile()
{
    SubroutineBlock->Compile();
}

void Subroutine::CompileReturn(uint16_t EndMagic)
{
    Node::Compile(EndMagic, NumEndParams);
}

void Program::Compile()
{
    for (auto i = Subroutines.begin(); i != Subroutines.end(); ++i)
        (*i)->Compile();
}

void Function::Compile()
{
    CompilePrototype(MAGIC_FUNCTION_DECLARATION, Arguments.size() + 1);
    for (auto i = Arguments.begin(); i != Arguments.end(); ++i)
        (*i)->CompileRaw();
    Subroutine::Compile();
    CompileReturn(MAGIC_END_FUNCTION);
}

void Chapter::Compile()
{
    CompilePrototype(MAGIC_CHAPTER_DECLARATION, 1);
    Subroutine::Compile();
    CompileReturn(MAGIC_END_CHAPTER);
}

void Scene::Compile()
{
    CompilePrototype(MAGIC_SCENE_DECLARATION, 1);
    Subroutine::Compile();
    CompileReturn(MAGIC_END_SCENE);
}

void Assignment::Compile()
{
    if (Name->Type == ARG_ARRAY)
        Name->Compile();
    Rhs->Compile();
    Node::Compile(Magic, 1);
    Name->CompileRaw();
    Node::Compile(MAGIC_CLEAR_PARAMS, 0);
}

void BinaryOperator::Compile()
{
    Lhs->Compile();
    Rhs->Compile();
    Node::Compile(Magic, 0);
}

void UnaryOperator::Compile()
{
    Rhs->Compile();
    Node::Compile(Magic, 0);
}

void UnaryStatement::Compile()
{
    UnaryOperator::Compile();
    Node::Compile(MAGIC_CLEAR_PARAMS, 0);
}

void Condition::_Compile(Argument& EndSym)
{
    Expr->Compile();
    Node::Compile(Magic, 1);
    EndSym.CompileRaw();
    ConditionBlock->Compile();
}

void If::Compile()
{
    Argument ElseSym("label.if." + to_string(SymCounter++), ARG_STRING);
    Argument EndSym("label.if.end." + to_string(SymCounter++), ARG_STRING);
    Condition::_Compile(ElseSym);
    Node::Compile(MAGIC_JUMP, 1);
    EndSym.CompileRaw();
    WriteSymbol(ElseSym.Data);
    if (ElseBlock) ElseBlock->Compile();
    if (ElseIf) ElseIf->Compile();
    WriteSymbol(EndSym.Data);
}

void While::Compile()
{
    Argument BeginSym("label.while.begin." + to_string(SymCounter), ARG_STRING);
    Argument EndSym("label.while.end." + to_string(SymCounter++), ARG_STRING);
    WriteSymbol(BeginSym.Data);
    Condition::_Compile(EndSym);
    Node::Compile(MAGIC_WHILE_END, 0);
    Node::Compile(MAGIC_JUMP, 1);
    BeginSym.CompileRaw();
    WriteSymbol(EndSym.Data);
}

// ugly hack
static Argument SelEndSym;

void Select::Compile()
{
    Argument BeginSym("label.select.begin." + to_string(SymCounter), ARG_STRING);
    Argument EndSym("label.select.end." + to_string(SymCounter++), ARG_STRING);
    Node::Compile(MAGIC_SELECT, 1);
    EndSym.CompileRaw();
    WriteSymbol(BeginSym.Data);

    // pass the end symbol down the tree to case
    Argument Old = SelEndSym;
    SelEndSym = EndSym;
    SelectBlock->Compile();
    SelEndSym = Old;

    Node::Compile(MAGIC_SELECT_END, 0);
    Node::Compile(MAGIC_JUMP, 1);
    BeginSym.CompileRaw();
    WriteSymbol(EndSym.Data);
    Node::Compile(MAGIC_SELECT_BREAK_END, 0);
}

void Case::Compile()
{
    Argument Label(Name, ARG_STRING);
    Argument StartSym("label.case.start." + to_string(SymCounter), ARG_STRING);
    Argument EndSym("label.case.end." + to_string(SymCounter++), ARG_STRING);
    Node::Compile(MAGIC_CASE, 3);
    Label.CompileRaw();
    EndSym.CompileRaw();
    StartSym.CompileRaw();
    WriteSymbol(StartSym.Data);
    CaseBlock->Compile();
    Node::Compile(MAGIC_CASE_END, 1);
    Label.CompileRaw();
    Node::Compile(MAGIC_JUMP, 1);
    SelEndSym.CompileRaw();
    WriteSymbol(EndSym.Data);
}

void Label::Compile()
{
    // TODO
}


namespace
{

// Muramasa CHS sources tolerate several source-level quirks the original
// Nitro+ compiler accepted: missing trailing ';', '==' typos as statements,
// etc. Normalize them before parsing.
bool IsWordChar(char c) { return isalnum((unsigned char)c) || c == '_' || (unsigned char)c >= 0x80; }
bool StartsKeyword(const string& t)
{
    static const char* Kw[] = {"if", "else", "while", "select", "case", "function",
                               "chapter", "scene", "#", "include", "continuation"};
    for (const char* k : Kw)
    {
        size_t n = strlen(k);
        if (t.compare(0, n, k) == 0 && (k[0] == '#' || t.size() == n || !IsWordChar(t[n])))
            return true;
    }
    return false;
}

// line is decoration: only CJK bytes or the ASCII set _=-~+* / and ws (e.g. ====*/, stray text)
bool IsDecoLine(const string& t)
{
    for (size_t i = 0; i < t.size(); i++)
    {
        unsigned char ch = t[i];
        if (ch >= 0x80 || ch == ' ' || ch == '\t') continue;
        if (ch == '=' || ch == '_' || ch == '-' || ch == '~' || ch == '+' || ch == '*' || ch == '/')
            continue;
        return false;
    }
    return true;
}

string SanitizeNss(const char* pBuffer, uint32_t Length)
{
    string src(pBuffer, Length);
    // normalize newlines for line-based processing
    string out;
    out.reserve(src.size() + 256);
    vector<string> lines;
    string cur;
    for (size_t i = 0; i < src.size(); i++)
    {
        if (src[i] == '\n') { lines.push_back(cur); cur.clear(); }
        else if (src[i] != '\r') cur += src[i];
    }
    if (!cur.empty()) lines.push_back(cur);

    bool inComment = false, inPre = false;
    int parenDepth = 0;
    int BraceDepth = 0;
    bool inSub = false;
    vector<bool> hasParenStmt;
    // find next non-empty code line index per position
    auto nextCodeLine = [&](size_t i, bool cSt, bool pSt) -> int
    {
        for (size_t j = i + 1; j < lines.size(); j++)
        {
            const string& t = lines[j];
            // strip leading ws
            size_t s = t.find_first_not_of(" \t\xE3\x80"); // includes fullwidth space bytes
            if (s == string::npos) continue;
            if (pSt || cSt) { /* still inside PRE/comment - treat via raw text */ }
            // skip pure comment lines
            string v = t.substr(s);
            if (v.rfind("//", 0) == 0 || v.rfind("..", 0) == 0 || v.rfind(".//", 0) == 0 ||
                v.rfind("+//", 0) == 0 || v.rfind("/*", 0) == 0 || v.rfind("*", 0) == 0)
                continue;
            return (int)j;
        }
        return -1;
    };

    for (size_t i = 0; i < lines.size(); i++)
    {
        string line = lines[i];
        string trimmed = line;
        size_t sp = trimmed.find_first_not_of(" \t\xE3\x80");
        trimmed = sp == string::npos ? "" : trimmed.substr(sp);

        if (inComment)
        {
            bool inStr2 = false;
            size_t close = string::npos;
            for (size_t k = 0; k + 1 < line.size(); k++)
            {
                if (line[k] == '"') inStr2 = !inStr2;
                else if (!inStr2 && line[k] == '*' && line[k+1] == '/')
                {
                    for (size_t m = k; m + 1 < line.size(); m++)
                    {
                        if (line[m] == '"') inStr2 = !inStr2;
                        else if (!inStr2 && line[m] == '*' && line[m+1] == '/') { close = m; goto closeFound; }
                    }
                }
            }
closeFound:
            // neutralize '*/' sequences inside quoted strings
            if (close == string::npos)
            {
                bool inQ = false;
                for (size_t k = 0; k + 1 < line.size(); k++)
                {
                    if (line[k] == '"') inQ = !inQ;
                    else if (inQ && line[k] == '*' && line[k+1] == '/')
                        line.insert(k + 1, 1, ' ');
                }
            }
            if (close != string::npos)
            {
                inComment = false;
                string rest = line.substr(close + 2);
                size_t rs = rest.find_first_not_of(" \t\xE3\x80");
                if (rs == string::npos || IsDecoLine(rest.substr(rs)))
                    line = line.substr(0, close + 2);
            }
            out += line + "\n";
            continue;
        }
        if (inPre)
        {
            if (trimmed == "</PRE>") inPre = false;
            if (getenv("NSS_TRACE")) fprintf(stderr, "L%d [INPRE] | %.60s\n", (int)i, trimmed.c_str());
            out += line + "\n";
            continue;
        }
        // statements outside any chapter/scene/function are unreachable:
        // comment them out before any region state can be entered
        if (!inSub)
        {
            bool isDecl = trimmed.rfind("function", 0) == 0 || trimmed.rfind("chapter", 0) == 0 ||
                          trimmed.rfind("scene", 0) == 0;
            if (isDecl) inSub = true;
            else if (!trimmed.empty() && trimmed[0] != '/' && trimmed.rfind("#include", 0) != 0)
            {
                out += "//" + line + "\n";
                continue;
            }
        }

        if (trimmed.rfind("/*", 0) == 0)
        {
            if (trimmed.find("*/") == string::npos) inComment = true;
            out += line + "\n";
            continue;
        }
        if (trimmed.rfind("<PRE", 0) == 0)
        {
            if (trimmed.find("</PRE>") == string::npos) inPre = true;
            out += line + "\n";
            continue;
        }
        if (trimmed.rfind("<voice", 0) == 0)
        {
            inPre = true; // text region ends at </PRE>
            out += line + "\n";
            continue;
        }

        // strip // comment (outside quotes) for analysis
        string code = line;
        bool inStr = false;
        for (size_t k = 0; k + 1 < code.size(); k++)
        {
            if (code[k] == '"') inStr = !inStr;
            else if (!inStr && (unsigned char)code[k] == 0xE3 && k + 2 < code.size() &&
                     (unsigned char)code[k+1] == 0x80 && (unsigned char)code[k+2] == 0x80)
            {   // U+3000 ideographic space glues to identifiers; normalize outside strings
                code[k] = ' ';
                code.erase(k + 1, 2);
                continue;
            }
            else if (!inStr && code[k] == '/' && code[k+1] == '/') { code = code.substr(0, k); break; }
        }
        line = code; // comments carry no semantics; all checks below use 'code'
        string ct = code;
        size_t tp = ct.find_first_not_of(" \t\xE3\x80");
        ct = tp == string::npos ? "" : ct.substr(tp);
        // trailing trim
        size_t te = ct.find_last_not_of(" \t\xE3\x80");
        if (te != string::npos) ct = ct.substr(0, te + 1);

        // orphan '*/' closer (its '/*' was inside a // or . comment): strip from code
        if (!inComment && !inPre)
        {
            bool inS = false;
            size_t ostar = string::npos, oslash = string::npos;
            for (size_t k = 0; k + 1 < code.size(); k++)
            {
                if (code[k] == '"') inS = !inS;
                else if (!inS && code[k] == '/' && code[k+1] == '*') oslash = k;
                else if (!inS && code[k] == '*' && code[k+1] == '/' && ostar == string::npos) ostar = k;
            }
            if (ostar != string::npos && oslash == string::npos)
            {
                code.erase(ostar, 2);
                tp = code.find_last_not_of(" \t\xE3\x80");
                ct = code;
                size_t s2 = ct.find_first_not_of(" \t\xE3\x80");
                ct = s2 == string::npos ? "" : ct.substr(0, tp == string::npos ? string::npos : tp + 1);
                ct = ct.substr(s2 == string::npos ? ct.size() : s2 - (ct.size() - (tp == string::npos ? 0 : tp + 1)));
                // simpler: recompute trimmed
                ct = code;
                s2 = ct.find_first_not_of(" \t\xE3\x80");
                ct = s2 == string::npos ? "" : ct.substr(s2);
                size_t e2 = ct.find_last_not_of(" \t\xE3\x80");
                if (e2 != string::npos) ct = ct.substr(0, e2 + 1);
                line = code;
            }
        }
        // decoration lines (==== */, stray CJK text): drop
        if (!ct.empty() && IsDecoLine(ct))
            { out += "\n"; continue; }
        // dangling operator before ';'  (".."+;  ->  "..";)
        if (!ct.empty() && ct.back() == ';')
        {
            size_t endp = line.find_last_not_of(" \t");
            if (endp != string::npos && line[endp] == ';' && endp > 0 &&
                (line[endp-1] == '+' || line[endp-1] == '-' || line[endp-1] == ','))
            {
                if (!(endp >= 2 && line[endp-1] == '+' && line[endp-2] == '+') &&
                    !(endp >= 2 && line[endp-1] == '-' && line[endp-2] == '-'))
                    line.erase(endp - 1, 1);
            }
        }
        // recompute ct after edits
        ct = line;
        {
            size_t s3 = ct.find_first_not_of(" \t\xE3\x80");
            ct = s3 == string::npos ? "" : ct.substr(s3);
            size_t e3 = ct.find_last_not_of(" \t\xE3\x80");
            if (e3 != string::npos) ct = ct.substr(0, e3 + 1);
        }

        {
            int pops = 0, pcl = 0;
            bool inS3 = false;
            for (size_t k = 0; k < ct.size(); k++)
            {
                if (ct[k] == '"') inS3 = !inS3;
                else if (!inS3 && ct[k] == '(') pops++;
                else if (!inS3 && ct[k] == ')') pcl++;
            }
            int allowed = (parenDepth > 0 ? parenDepth : 0) + pops;
            int surplusP = pcl - allowed;
            while (surplusP > 0)
            {
                size_t b = line.find_last_of(')');
                if (b == string::npos) break;
                line.erase(b, 1);
                surplusP--;
                pcl--;
            }
        }


        if (parenDepth <= 0 && !ct.empty())
        {
            // '==' typo as statement: "$x == v;" -> "$x = v;"
            if ((ct[0] == '$' || ct[0] == '#' || ct[0] == '@') && ct.find('(') == string::npos &&
                ct.find("if") != 0)
            {
                size_t eq = ct.find("==");
                if (eq != string::npos && ct.find('=', eq + 2) == string::npos)
                {
                    size_t pos = line.find("==");
                    if (pos != string::npos) line.replace(pos, 2, "=");
                }
            }
            // missing ';'
            if (!StartsKeyword(ct) && ct.back() != ';' && ct.back() != '{' && ct.back() != '}' &&
                ct.back() != ':' && ct.back() != ',' && ct.back() != '(')
            {
                char c = ct.back();
                bool endsPair = (c == '+' && ct.size() >= 2 && ct[ct.size() - 2] == '+') ||
                                (c == '-' && ct.size() >= 2 && ct[ct.size() - 2] == '-');
                bool trigger = c == ')' || c == ']' || c == '"' || endsPair || IsWordChar(c);
                if (trigger)
                {
                    int nxt = nextCodeLine(i, inComment, inPre);
                    bool add = true;
                    if (nxt >= 0)
                    {
                        string nt = lines[nxt];
                        size_t ns = nt.find_first_not_of(" \t\xE3\x80");
                        nt = nt.substr(ns);
                        char d = nt[0];
                        static const string stop = "{,;)] +-*/%<>=!&|:?.@";
                        if (stop.find(d) != string::npos) add = false;
                        if (nt.rfind("else", 0) == 0 || nt.rfind("case", 0) == 0) add = false;
                    }
                    if (add)
                    {
                        // insert ; at end of code part (before comment)
                        line = code;
                        size_t le = line.find_last_not_of(" \t");
                        if (le != string::npos) { line = line.substr(0, le + 1) + ";"; }
                    }
                }
            }
        }
        // update paren depth on code part
        inStr = false;
        int opens = 0, closes = 0;
        for (size_t k = 0; k < line.size(); k++)
        {
            if (line[k] == '"') inStr = !inStr;
            else if (!inStr && line[k] == '(') parenDepth++;
            else if (!inStr && line[k] == ')') parenDepth--;
            else if (!inStr && line[k] == '{') opens++;
            else if (!inStr && line[k] == '}') closes++;
        }
        // orphan '}' (would take depth negative): drop surplus closers
        int depthAfter = BraceDepth + opens - closes;
        if (depthAfter < 0)
        {
            int surplus = -depthAfter;
            // remove up to 'surplus' '}' chars from line (rightmost-first)
            while (surplus > 0)
            {
                size_t b = line.find_last_of('}');
                if (b == string::npos) break;
                line.erase(b, 1);
                surplus--;
            }
            closes -= -depthAfter;
            depthAfter = BraceDepth + opens - closes;
        }
        if (inSub && depthAfter <= 0 && BraceDepth > 0) inSub = false;
        BraceDepth = depthAfter;
        if (getenv("NSS_TRACE")) fprintf(stderr, "L%d inSub=%d inPre=%d inCmt=%d | %.60s\n", (int)i, (int)inSub, (int)inPre, (int)inComment, ct.c_str());
        out += line + "\n";
    }
    while (BraceDepth-- > 0) out += "}\n";
    return out;
}

}

namespace Nss
{

void Compile(const char* pBuffer, uint32_t Length, Buffer* NsbBuffer, Buffer* MapBuffer)
{
    Counter = 1;
    SymCounter = 1;
    Output = NsbBuffer;
    MapOutput = MapBuffer;
    string Sanitized = SanitizeNss(pBuffer, Length);
    if (getenv("NSS_DEBUG_SAN"))
    {
        FILE* f = std::fopen(getenv("NSS_DEBUG_SAN"), "wb");
        if (f) { std::fwrite(Sanitized.data(), 1, Sanitized.size(), f); std::fclose(f); }
    }
    YY_BUFFER_STATE buffer = yy_scan_bytes(Sanitized.data(), Sanitized.size());
    yyparse();
    pRoot->Compile();
    yy_delete_buffer(buffer);
    delete pRoot;
}

}
