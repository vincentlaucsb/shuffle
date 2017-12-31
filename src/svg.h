#pragma once
#include <math.h> // NAN
#include <unordered_map>
#include <map>
#include <deque>
#include <vector>
#include <string>
#include <memory>

using std::vector;
using std::string;

namespace SVG {
    class Element {
    public:
        Element() {};
        Element(
            std::string _tag,
            std::map < std::string, std::string > _attr) :
            attr(_attr),
            tag(_tag) {};
            
        template<typename T>
        inline Element& set_attr(std::string key, T value) {
            this->attr[key] = std::to_string(value);
            return *this;
        }
            
        template<typename T, typename... Args>
        inline void add_child(T node, Args... args) {
            add_child(node);
            add_child(args...);
        }
        
        template<typename T>
        inline Element* add_child(T node) {
            /** Also return a pointer to the element added */
            this->children.push_back(std::make_shared<Element>(node));
            return this->children.back().get();
        }

        std::string to_string();
        std::map < std::string, std::string > attr;
        std::string content;

    protected:
        std::string tag;
        std::vector<std::shared_ptr<Element>> children;
    };

    template<>
    inline Element& Element::set_attr(std::string key, const char * value) {
        this->attr[key] = value;
        return *this;
    }

    template<>
    inline Element& Element::set_attr(std::string key, const std::string value) {
        this->attr[key] = value;
        return *this;
    }
    
    class SVG : public Element {
    public:
        SVG(std::map < std::string, std::string > _attr=
        { { "xmlns", "http://www.w3.org/2000/svg" } }
        ) : Element("svg", _attr) {};
    };

    class Text : public Element {
    public:
        Text() { tag = "text"; };
        Text(int x, int y, std::string _content) {
            tag = "text";
            set_attr("x", std::to_string(x));
            set_attr("y", std::to_string(y));
            content = _content;
        }
    };

    class Group : public Element {
    public:
        Group() : Element("g", {}) {};
    };

    class Line : public Element {
    public:
        Line(int x1, int x2, int y1, int y2) : Element("line", {
            { "x1", std::to_string(x1) },
            { "x2", std::to_string(x2) },
            { "y1", std::to_string(y1) },
            { "y2", std::to_string(y2) }
        }) {};
    };

    class Rect : public Element {
    public:
        Rect(
            int x, int y, double width, double height) : 
            Element("rect", {
            { "x", std::to_string(x) },
            { "y", std::to_string(y) },
            { "width", std::to_string(width) },
            { "height", std::to_string(height) }
        }){};
    };

    class Circle : public Element {
    public:
        Circle(int cx, int cy, int radius) :
            Element("circle", {
                {"cx", std::to_string(cx)},
                {"cy", std::to_string(cy)},
                {"r", std::to_string(radius)}
            }) {};
    };
}

namespace Graphs {
    vector<bool> numeric_types(std::string filename, int nrows);

    struct GraphOptions {
        int width = 800;
        int height = 460;
    };

    const GraphOptions DEFAULT_GRAPH = GraphOptions();

    /** This provides a base class for any graph and provides common methods for
     *  drawing the x and y axes as well as positioning the main drawing area    
     *
     *  For the purposes of this library, the "margin" of a graph is defined as the
     *  area not containing the main graphical element, e.g. bars or points. The margin
     *  is the area which contains the title, subtitles, tick marks, etc. Thus, the main
     *  drawing area is the area not inside the margins. For convienience, any methods
     *  which create SVG elements may use the methods x1(), x2(), y1(), and y2()
     *  to get the boundaries of the drawing area.
     */
    class Graph {
    public:
        const enum x_lab_align { left, center };
        Graph(GraphOptions options=DEFAULT_GRAPH);
        virtual void generate() {};
        void to_svg(const std::string filename);

        SVG::SVG root;
        int width;
        int height;

    protected:
        SVG::Group make_x_axis(Graph::x_lab_align align = left);
        SVG::Group make_y_axis();

        inline int x1() {
            /** Return boundary of drawing area */
            return margin_left;
        }

        inline int x2() {
            /** Return boundary of drawing area */
            return width - margin_right;
        }

        inline int y1() {
            /** Return boundary of drawing area */
            return margin_top;
        }

        inline int y2() {
            /** Return boundary of drawing area */
            return height - margin_bottom;
        }

        inline double x_tick_space() {
            /** Return the space between n evenly spaced ticks
             *
             *  Can also used to determine size of bars for bar charts,
             *  in which case "ticks" is synonymous with "number of bars"
             */
            return (x2() - x1()) / n_ticks;
        }

        int margin_left = 50;    /*< Also includes space for y-axis labels */
        int margin_right = 50;
        int margin_bottom = 100; /*< Also includes space for x-axis labels */
        int margin_top = 50;     /*< Also includes space for title */

        // Used to map data values to SVG width/height values
        long double domain_min = NAN;
        long double domain_max = NAN;
        long double range_min = NAN; /*< Minimum value of data (y-axis) */
        long double range_max = NAN; /*< Maximum value of data (y-axis) */

        size_t n_ticks = 20;     /*< Also number of bins */
        int bar_spacing = 2;
        int tick_size = 5;

        vector<std::string> x_tick_labels;

        SVG::Element* title = nullptr; /*< Pointer set by constructor */
        SVG::Element* xlab = nullptr;
        SVG::Element* ylab = nullptr;
    };

    class BarChart : public Graph {
    public:
        BarChart(GraphOptions options = DEFAULT_GRAPH) : Graph(options) {};
        BarChart(const string filename, const string col_x, const string col_y,
            const std::unordered_map<string, string> options,
            const GraphOptions options2 = DEFAULT_GRAPH);
        void generate() override;

    protected:
        SVG::Group make_bars();

        SVG::Group bars;
        vector<long double> values;
    };

    class Histogram: public BarChart {
    public:
        Histogram(const string filename, const string col_name, const string title="",
            const string x_lab = "", const string y_lab = "", const size_t bins = 20,
            const GraphOptions options = DEFAULT_GRAPH);
        void generate() override;
    };

    class Scatterplot: public Graph {
    public:
        Scatterplot(const string filename, const string col1, const string col2,
            const string title, const GraphOptions options=DEFAULT_GRAPH);
        void generate() override;

    protected:
        SVG::Group make_dots();

        SVG::Group dots;
        std::deque<vector<long double>> points;
    };

    /**
     * Class for multi-graph SVGs
     */
    class Matrix {
    public:
        Matrix(int _cols) : cols(_cols) {};
        void add_graph(Graph graph);
        void generate();
        void to_svg(const string filename);

    private:
        SVG::SVG root;
        std::deque<Graph> graphs;
        int cols;
        int width_per_graph = 500;
        int height_per_graph = 400;
    };

    void matrix_hist(const std::string filename, const std::string outfile);

    class ColumnNotFoundError : public std::runtime_error {
    public:
        ColumnNotFoundError(const std::string& col_name) : std::runtime_error(
            "Couldn't find a column named " + col_name) {};
    };
}