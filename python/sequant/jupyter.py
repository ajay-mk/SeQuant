"""
Jupyter Notebook Utilities
===========================

Helper functions for using SeQuant in Jupyter notebooks.

Functions
---------
display_eqs(eqs, labels=None)
    Display multiple equations with optional labels
compare_eqs(eq1, eq2, label1="Left", label2="Right")
    Display two equations side by side for comparison
"""

try:
    from IPython.display import display, Markdown, Latex, HTML
    IPYTHON_AVAILABLE = True
except ImportError:
    IPYTHON_AVAILABLE = False


def display_eqs(eqs, labels=None):
    """
    Display multiple equations in a Jupyter notebook.

    Parameters
    ----------
    eqs : list of ExprPtr or dict
        Equations to display. Can be a list of expressions or a dict
        mapping labels to expressions.
    labels : list of str, optional
        Labels for each equation. If not provided and eqs is a dict,
        uses the dict keys. Otherwise uses generic labels.

    Examples
    --------
    >>> from sequant.mbpt import CC
    >>> from sequant.jupyter import display_eqs
    >>> cc = CC(2)
    >>> t_eqs = cc.t()
    >>> display_eqs({"T1": t_eqs[1], "T2": t_eqs[2]})

    Or with a list:
    >>> display_eqs([t_eqs[1], t_eqs[2]], labels=["T1 equation", "T2 equation"])
    """
    if not IPYTHON_AVAILABLE:
        print("IPython not available. Install jupyter to use this function.")
        return

    # Handle dict input
    if isinstance(eqs, dict):
        labels = list(eqs.keys())
        eqs = list(eqs.values())

    # Generate default labels if not provided
    if labels is None:
        labels = [f"Equation {i+1}" for i in range(len(eqs))]

    # Build HTML output
    html_parts = ['<div style="margin: 20px 0;">']

    for label, eq in zip(labels, eqs):
        latex_str = eq.latex if hasattr(eq, 'latex') else str(eq)
        html_parts.append(f'''
        <div style="margin: 15px 0; padding: 10px; border-left: 3px solid #4CAF50;">
            <div style="font-weight: bold; margin-bottom: 5px; color: #333;">{label}</div>
            <div style="font-size: 14px;">$$
                {latex_str}
            $$</div>
        </div>
        ''')

    html_parts.append('</div>')
    display(HTML(''.join(html_parts)))


def compare_eqs(eq1, eq2, label1="Expression 1", label2="Expression 2"):
    """
    Display two equations side by side for comparison.

    Parameters
    ----------
    eq1 : ExprPtr
        First equation
    eq2 : ExprPtr
        Second equation
    label1 : str, optional
        Label for first equation
    label2 : str, optional
        Label for second equation

    Examples
    --------
    >>> from sequant import simplify, canonicalize
    >>> from sequant.jupyter import compare_eqs
    >>> expr = ...  # some expression
    >>> simplified = simplify(expr)
    >>> compare_eqs(expr, simplified, "Original", "Simplified")
    """
    if not IPYTHON_AVAILABLE:
        print("IPython not available. Install jupyter to use this function.")
        return

    latex1 = eq1.latex if hasattr(eq1, 'latex') else str(eq1)
    latex2 = eq2.latex if hasattr(eq2, 'latex') else str(eq2)

    html = f'''
    <div style="display: flex; gap: 20px; margin: 20px 0;">
        <div style="flex: 1; padding: 15px; border: 1px solid #ddd; border-radius: 5px;">
            <div style="font-weight: bold; margin-bottom: 10px; color: #2196F3;">{label1}</div>
            <div style="font-size: 14px;">$$
                {latex1}
            $$</div>
        </div>
        <div style="flex: 1; padding: 15px; border: 1px solid #ddd; border-radius: 5px;">
            <div style="font-weight: bold; margin-bottom: 10px; color: #4CAF50;">{label2}</div>
            <div style="font-size: 14px;">$$
                {latex2}
            $$</div>
        </div>
    </div>
    '''
    display(HTML(html))


def show_context():
    """
    Display current SeQuant context settings in a formatted table.

    Examples
    --------
    >>> import sequant as sq
    >>> from sequant.jupyter import show_context
    >>> sq.set_vacuum(sq.Vacuum.SingleProduct)
    >>> sq.set_spbasis(sq.SPBasis.Spinfree)
    >>> show_context()
    """
    if not IPYTHON_AVAILABLE:
        print("IPython not available. Install jupyter to use this function.")
        return

    import sequant as sq
    from sequant.mbpt import get_csv

    vacuum = sq.get_vacuum()
    spbasis = sq.get_spbasis()
    csv = get_csv()

    html = f'''
    <div style="margin: 20px 0; padding: 15px; background-color: #f5f5f5; border-radius: 5px;">
        <h4 style="margin-top: 0; color: #333;">SeQuant Context Settings</h4>
        <table style="width: 100%; border-collapse: collapse;">
            <tr style="border-bottom: 1px solid #ddd;">
                <td style="padding: 8px; font-weight: bold;">Vacuum:</td>
                <td style="padding: 8px;">{vacuum}</td>
            </tr>
            <tr style="border-bottom: 1px solid #ddd;">
                <td style="padding: 8px; font-weight: bold;">SP Basis:</td>
                <td style="padding: 8px;">{spbasis}</td>
            </tr>
            <tr>
                <td style="padding: 8px; font-weight: bold;">CSV:</td>
                <td style="padding: 8px;">{csv}</td>
            </tr>
        </table>
    </div>
    '''
    display(HTML(html))


__all__ = ['display_eqs', 'compare_eqs', 'show_context']
